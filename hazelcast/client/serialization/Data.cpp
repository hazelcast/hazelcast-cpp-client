//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Data.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "ConstantSerializers.h"


namespace hazelcast {
    namespace client {
        namespace serialization {

            Data::Data() : partitionHash(-1)
            , buffer(0)
            , type(SerializationConstants::CONSTANT_TYPE_DATA) {

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


            void Data::setSerializationContext(SerializationContext *context) {
                this->context = context;
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
                total += 4; // id
                if (cd != NULL) {
                    total += 4; // classDefinition-classId
                    total += 4; // classDefinition-namespace-size
                    total += 4; // // classDefinition-factory-id
                    total += 4; // classDefinition-version
                    total += 4; // classDefinition-binary-length
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

            int Data::getFactoryId() const {
                return FACTORY_ID;
            };

            int Data::getClassId() const {
                return ID;
            };

            void Data::writeData(BufferedDataOutput & dataOutput) const {
                dataOutput << type;
                if (cd != NULL) {
                    dataOutput.writeInt(cd->getClassId());
                    dataOutput.writeInt(cd->getFactoryId());
                    dataOutput.writeInt(cd->getVersion());
                    std::vector<byte> classDefBytes = cd->getBinary();
                    dataOutput.writeInt(classDefBytes.size());
                    dataOutput.write(classDefBytes);
                } else {
                    dataOutput.writeInt(NO_CLASS_ID);
                }
                int len = bufferSize();
                dataOutput.writeInt(len);
                if (len > 0) {
                    dataOutput.write(buffer);
                }
                dataOutput.writeInt(partitionHash);

            }

            void Data::readData(BufferedDataInput & dataInput) {
                dataInput >> type;
                int classId = NO_CLASS_ID;
                dataInput >> classId;
                if (classId != NO_CLASS_ID) {
                    int factoryId = 0;
                    int version = 0;
                    dataInput >> factoryId;
                    dataInput >> version;

                    int classDefSize = 0;
                    dataInput >> classDefSize;

                    if (context->isClassDefinitionExists(factoryId, classId, version)) {
                        cd = context->lookup(factoryId, classId, version);
                        dataInput.skipBytes(classDefSize);//TODO msk ???
                    } else {
                        std::vector<byte> classDefBytes(classDefSize);
                        dataInput >> classDefBytes;
                        cd = context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = 0;
                dataInput >> size;
                if (size > 0) {
                    std::vector<byte> buffer(size);
                    dataInput >> buffer;
                    this->buffer = buffer;
                }
                dataInput >> partitionHash;
            }


        }
    }
}

