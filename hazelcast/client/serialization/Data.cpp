//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Data.h"
#include "DataInput.h"
#include "SerializationContext.h"
#include "ClassDefinition.h"


namespace hazelcast {
    namespace client {
        namespace serialization {

            Data::Data() : partitionHash(-1)
            , buffer(0)
            , type(-1) {

            };

            Data::Data(const Data& rhs) {
                (*this) = rhs;
            };

            Data::Data(const int type, std::vector<byte> buffer) : partitionHash(-1) {
                this->type = type;
                this->buffer = buffer;
            };

            Data::~Data() {
            };

            Data& Data::operator = (const Data& rhs) {
                type = rhs.type;
                buffer = rhs.buffer;
                cd = rhs.cd;
                partitionHash = rhs.partitionHash;
                return (*this);
            };

            bool Data::operator ==(const Data& rhs) const {
                if (type != rhs.type) return false;
                if (cd != rhs.cd) return false;
                if (partitionHash != rhs.partitionHash) return false;
                if (buffer != rhs.buffer) return false;
                return true;
            };

            bool Data::operator !=(const Data& rhs) const {
                return !((*this) == rhs);
            };

            int Data::bufferSize() const {
                return buffer.size();
            };

            /**
             * Calculates the size of the binary after the Data is serialized.
             *
             * WARNING:
             *
             * Should be in sync with {@link #writeData(com.hazelcast.nio.ObjectDataOutput)}
             */
            int Data::totalSize() const {
                int total = 0;
                total += 4; // type
                if (cd != NULL) {
                    total += 4; // cd-classId
                    total += 4; // cd-version
                    total += 4; // cd-binary-length
                    total += cd->getBinary().size(); // cd-binary
                } else {
                    total += 4; // no-classId
                }
                total += 4; // buffer-size
                total += bufferSize(); // buffer
                total += 4; // partition-hash
                return total;
            };

            int Data::getPartitionHash() {
                return partitionHash;
            };

            void Data::setPartitionHash(int partitionHash) {
                this->partitionHash = partitionHash;
            };

            void Data::readData(DataInput& in) {
                type = in.readInt();
                int classId = in.readInt();
                if (classId != NO_CLASS_ID) {
                    int version = in.readInt();
                    SerializationContext *context = in.getSerializationContext();

                    int classDefSize = in.readInt();

                    if (context->isClassDefinitionExists(classId, version)) {
                        cd = context->lookup(classId, version);
                        in.skipBytes(classDefSize);
                    } else {
                        std::vector<byte> classDefBytes(classDefSize);
                        in.readFully(classDefBytes);
                        cd = context->createClassDefinition(classDefBytes);
                    }
                }
                int size = in.readInt();
                if (size > 0) {
                    std::vector<byte> buffer(size);
                    in.readFully(buffer);
                    this->buffer = buffer;
                }
                partitionHash = in.readInt();
            };

            void Data::writeData(DataOutput& out) const {
                out.writeInt(type);
                if (cd != NULL) {
                    out.writeInt(cd->getClassId());
                    out.writeInt(cd->getVersion());
                    std::vector<byte> classDefBytes = cd->getBinary();
                    out.writeInt(classDefBytes.size());
                    out.write(classDefBytes);//TODO only usage ins class this necessary
                } else {
                    out.writeInt(NO_CLASS_ID);
                }
                int len = bufferSize();
                out.writeInt(len);
                if (len > 0) {
                    out.write(buffer); //TODO only usage ins class this necessary
                }
                out.writeInt(partitionHash);
            };

        }
    }
}

