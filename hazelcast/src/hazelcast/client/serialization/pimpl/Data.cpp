//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                Data::Data()
                : partitionHash(0)
                , type(SerializationConstants::CONSTANT_TYPE_DATA)
                , buffer(new std::vector<byte>) {

                };

                Data::Data(const Data &rhs)
                : partitionHash(rhs.partitionHash)
                , type(rhs.type)
                , cd(rhs.cd)
                , buffer(rhs.buffer.release()) {

                };

                Data &Data::operator = (const Data &rhs) {
                    partitionHash = rhs.partitionHash;
                    type = rhs.type;
                    cd = rhs.cd;
                    buffer = rhs.buffer;
                    return (*this);
                };

                int Data::bufferSize() const {
                    return buffer->size();
                };

                /**
                 * Calculates the size of the binary after the Data is serialized.
                 *
                 * WARNING:
                 *
                 * Should be in sync with #writeData(com.hazelcast.nio.ObjectDataOutput)
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


                int Data::hashCode() const {
                    if (buffer.get() == NULL)
                        return 0;
                    // FNV (Fowler/Noll/Vo) Hash "1a"
                    const int prime = 0x01000193;
                    int hash = 0x811c9dc5;
                    for (int i = buffer->size() - 1; i >= 0; i--) {
                        hash = (hash ^ (*buffer)[i]) * prime;
                    }
                    return hash;
                };

                int Data::getPartitionHash() const {
                    if (partitionHash == 0) {
                        if (buffer.get() != NULL) {
                            partitionHash = hashCode();
                        }
                    }
                    return partitionHash;
                };


                void Data::setPartitionHash(int partitionHash) {
                    this->partitionHash = partitionHash;
                }

                int Data::getType() const {
                    return type;
                };

                void Data::setType(int type) {
                    this->type = type;
                };

                void Data::setBuffer(std::auto_ptr<std::vector<unsigned char> > buffer) {
                    this->buffer = buffer;
                };


                int Data::getFactoryId() const {
                    return FACTORY_ID;
                }

                int Data::getClassId() const {
                    return CLASS_ID;
                }

                void Data::writeData(serialization::ObjectDataOutput &objectDataOutput) const {
                    objectDataOutput.writeInt(type);
                    if (cd != NULL) {
                        objectDataOutput.writeInt(cd->getClassId());
                        objectDataOutput.writeInt(cd->getFactoryId());
                        objectDataOutput.writeInt(cd->getVersion());
                        const std::vector<byte> &classDefBytes = cd->getBinary();

                        objectDataOutput.writeInt(classDefBytes.size());
                        objectDataOutput.write(classDefBytes);
                    } else {
                        objectDataOutput.writeInt(NO_CLASS_ID);
                    }
                    int len = bufferSize();
                    objectDataOutput.writeInt(len);
                    if (len > 0) {
                        objectDataOutput.write(*(buffer.get()));
                    }
                    objectDataOutput.writeInt(partitionHash);
                }

                void Data::readData(serialization::ObjectDataInput &objectDataInput) {
                    type = objectDataInput.readInt();
                    int classId = objectDataInput.readInt();

                    if (classId != NO_CLASS_ID) {
                        int factoryId = objectDataInput.readInt();
                        int version = objectDataInput.readInt();

                        int classDefSize = objectDataInput.readInt();
                        SerializationContext *serializationContext = objectDataInput.getSerializationContext();
                        if (serializationContext->isClassDefinitionExists(factoryId, classId, version)) {
                            cd = serializationContext->lookup(factoryId, classId, version);
                            objectDataInput.skipBytes(classDefSize);
                        } else {
                            std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                            objectDataInput.readFully(*(classDefBytes.get()));
                            cd = serializationContext->createClassDefinition(factoryId, classDefBytes);
                        }
                    }
                    int size = objectDataInput.readInt();
                    if (size > 0) {
                        this->buffer->resize(size);
                        objectDataInput.readFully(*(buffer.get()));
                    }
                    partitionHash = objectDataInput.readInt();
                }

            }
        }
    }
}

