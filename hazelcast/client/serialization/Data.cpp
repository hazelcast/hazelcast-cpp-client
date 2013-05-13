//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Data.h"
#include "SerializationContext.h"
#include "ClassDefinition.h"
#include "SerializationConstants.h"


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

            template<typename DataOutput>
            void operator <<(DataOutput& dataOutput, const Data& data) {
                dataOutput << data.type;
                if (data.cd != NULL) {
                    dataOutput << data.cd->getClassId();
                    dataOutput << data.cd->getFactoryId();
                    dataOutput << data.cd->getVersion();
                    std::vector<byte> classDefBytes = data.cd->getBinary();
                    dataOutput << classDefBytes.size();
                    dataOutput << classDefBytes;
                } else {
                    dataOutput << data.NO_CLASS_ID;
                }
                int len = data.bufferSize();
                dataOutput << len;
                if (len > 0) {
                    dataOutput << data.buffer;
                }
                dataOutput << data.partitionHash;
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, Data& data) {
                dataInput >> data.type;
                int classId = data.NO_CLASS_ID;
                dataInput >> data;
                if (classId != data.NO_CLASS_ID) {
                    int factoryId = 0;
                    int version = 0;
                    dataInput >> factoryId;
                    dataInput >> version;

                    int classDefSize = 0;
                    dataInput >> classDefSize;

                    if (data.context->isClassDefinitionExists(factoryId, classId, version)) {
                        data.cd = data.context->lookup(factoryId, classId, version);
                        dataInput.skipBytes(classDefSize);//TODO msk ???
                    } else {
                        std::vector<byte> classDefBytes(classDefSize);
                        dataInput >> classDefBytes;
                        data.cd = data.context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = 0;
                dataInput >> size;
                if (size > 0) {
                    std::vector<byte> buffer(size);
                    dataInput >> buffer;
                    data.buffer = buffer;
                }
                dataInput >> data.partitionHash;
            };
        }
    }
}

