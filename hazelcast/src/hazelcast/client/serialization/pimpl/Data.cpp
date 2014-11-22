//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/MurmurHash3.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int Data::HEADER_ENTRY_LENGTH = 12;
                int Data::HEADER_FACTORY_OFFSET = 0;
                int Data::HEADER_CLASS_OFFSET = 4;
                int Data::HEADER_VERSION_OFFSET = 8;

                Data::Data()
                : data(new std::vector<byte>)
                , header(new std::vector<byte>)
                , partitionHash(0)
                , type(SerializationConstants::CONSTANT_TYPE_DATA) {

                }

                Data::Data(const Data& rhs)
                : data(rhs.data.release())
                , header(rhs.header.release())
                , partitionHash(rhs.partitionHash)
                , type(rhs.type) {

                }

                Data& Data::operator=(const Data& rhs) {
                    partitionHash = rhs.partitionHash;
                    type = rhs.type;
                    data = rhs.data;
                    header = rhs.header;
                    return (*this);
                }

                int Data::bufferSize() const {
                    return data->size();
                }


                size_t Data::headerSize() const {
                    return header->size();
                }

                int Data::hashCode() const {
                    return hazelcast::util::MurmurHash3_x86_32((void*)&((*data)[0]) , data->size());
                }


                bool Data::hasPartitionHash() const {
                    return partitionHash != 0;
                }

                int Data::getPartitionHash() const {
                    if (partitionHash == 0) {
                        partitionHash = hashCode();
                    }
                    return partitionHash;
                }


                void Data::setPartitionHash(int partitionHash) {
                    this->partitionHash = partitionHash;
                }

                int Data::getType() const {
                    return type;
                }

                void Data::setType(int type) {
                    this->type = type;
                }

                void Data::setBuffer(std::auto_ptr<std::vector<byte> > buffer) {
                    this->data = buffer;
                }

                void Data::setHeader(std::auto_ptr<std::vector<byte> > header) {
                    this->header = header;
                }

                bool Data::isPortable() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE == type;
                }


                bool Data::hasClassDefinition() const {
                    if (isPortable()) {
                        return true;
                    }
                    return headerSize() > 0;
                }


                size_t Data::getClassDefinitionCount() const {
                    size_t len = headerSize();
                    assert(len % HEADER_ENTRY_LENGTH == 0 && "Header length should be factor of HEADER_ENTRY_LENGTH");
                    return len / HEADER_ENTRY_LENGTH;
                }

                std::vector<boost::shared_ptr<ClassDefinition> > Data::getClassDefinitions(PortableContext& context) const{
                    if (headerSize() == 0) {
                        return std::vector<boost::shared_ptr<ClassDefinition> >();
                    }

                    size_t count = getClassDefinitionCount();
                    std::vector<boost::shared_ptr<ClassDefinition> > definitions(count);
                    for (int i = 0; i < count; i++) {
                        definitions[i] = readClassDefinition(context, i * Data::HEADER_ENTRY_LENGTH);
                    }
                    return definitions;
                }

                boost::shared_ptr<ClassDefinition> Data::readClassDefinition(PortableContext& context, int start) const{
                    int factoryId = readIntHeader(start + Data::HEADER_FACTORY_OFFSET);
                    int classId = readIntHeader(start + Data::HEADER_CLASS_OFFSET);
                    int version = readIntHeader(start + Data::HEADER_VERSION_OFFSET);
                    return context.lookup(factoryId, classId, version);
                }

                int Data::readIntHeader(int offset) const{
                    std::vector<byte>& b = *header;
                    int byte3 = (b[offset] & 0xFF) << 24;
                    int byte2 = (b[offset + 1] & 0xFF) << 16;
                    int byte1 = (b[offset + 2] & 0xFF) << 8;
                    int byte0 = b[offset + 3] & 0xFF;
                    return byte3 + byte2 + byte1 + byte0;
                }
            }
        }
    }
}


