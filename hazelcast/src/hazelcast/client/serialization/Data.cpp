//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Data.h"


namespace hazelcast {
    namespace client {
        namespace serialization {

            Data::Data()
            : partitionHash(-1)
            , type(SerializationConstants::CONSTANT_TYPE_DATA)
            , isError(false)
            , cd(NULL)
            , buffer(new std::vector<byte>) {

            };

            Data::Data(const Data& rhs)
            : partitionHash(rhs.partitionHash)
            , type(rhs.type)
            , isError(rhs.isError)
            , cd(rhs.cd)
            , buffer(rhs.buffer.release()) {

            };

            Data::Data(const int type, std::auto_ptr <std::vector<byte> > buffer) : partitionHash(-1)
            , isError(false)
            , cd(NULL)
            , buffer(buffer) {
                this->type = type;
            };

            Data::~Data() {
            };

            Data& Data::operator = (const Data& rhs) {
                partitionHash = rhs.partitionHash;
                type = rhs.type;
                isError = rhs.isError;
                cd = rhs.cd;
                buffer = rhs.buffer;
                return (*this);
            };

            bool Data::isServerError() const {
                return isError;
            };

            int Data::bufferSize() const {
                return buffer->size();
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


            int Data::hashCode() {
                if (buffer.get() == NULL) return INT_MIN;
                // FNV (Fowler/Noll/Vo) Hash "1a"
                const int prime = 0x01000193;
                int hash = 0x811c9dc5;
                for (int i = buffer->size() - 1; i >= 0; i--) {
                    hash = (hash ^ (*buffer)[i]) * prime;
                }
                return hash;
            };

            int Data::getPartitionHash() {
                if (partitionHash == -1) {
                    if (buffer.get() != NULL) {
                        partitionHash = hashCode();
                    }
                }
                return partitionHash;
            };

            void Data::setPartitionHash(int partitionHash) {
                this->partitionHash = partitionHash;
            };

            int Data::getType() const {
                return type;
            };

            void Data::setType(int type) {
                this->type = type;
            };

            void Data::setBuffer(auto_ptr<vector<unsigned char> > buffer) {
                this->buffer = buffer;
            };

            int Data::getFactoryId() const {
                return FACTORY_ID;
            };

            int Data::getClassId() const {
                return ID;
            };


        }
    }
}

