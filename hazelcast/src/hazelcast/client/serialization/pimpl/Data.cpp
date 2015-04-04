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
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Bits.h"

#include <stdio.h>
#include <algorithm>


using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                // type and partition_hash are always written with BIG_ENDIAN byte-order
                unsigned int Data::TYPE_OFFSET = 0;
                // will use a byte to store partition_hash bit
                unsigned int Data::PARTITION_HASH_BIT_OFFSET = 4;
                unsigned int Data::DATA_OFFSET = 5;

                Data::Data()
                : data(new std::vector<byte>()){
                }

                Data::Data(std::auto_ptr<std::vector<byte> > buffer) {
                    if (buffer.get() != 0 && buffer->size() > 0 && buffer->size() < DATA_OFFSET) {
                        char msg[100];
                        sprintf(msg, "Provided buffer should be either empty or "
                                "should contain more than %d bytes! Provided buffer size:%ld", DATA_OFFSET, buffer->size());
                        throw exception::IllegalArgumentException("Data::setBuffer", msg);
                    }
                    this->data = buffer;
                }

                Data::Data(const Data& rhs)
                : data(rhs.data) {

                }

                Data& Data::operator=(const Data& rhs) {
                    data = rhs.data;
                    return (*this);
                }


                size_t Data::dataSize() const {
                    return std::max<size_t>(totalSize() - DATA_OFFSET, (size_t)0);
                }

                size_t Data::totalSize() const {
                    return data.get() != 0 ? data->size() : 0;
                }

                int Data::getPartitionHash() const {
                    if (hasPartitionHash()) {
                        return Bits::readIntB(*data, data->size() - Bits::INT_SIZE_IN_BYTES);
                    }
                    return hashCode();
                }

                bool Data::hasPartitionHash() const {
                    return totalSize() != 0 && (*data)[PARTITION_HASH_BIT_OFFSET] != 0;
                }

                std::vector<byte>  &Data::toByteArray() const {
                    return *data;
                }

                int Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return Bits::readIntB(*data, TYPE_OFFSET);
                }

                int Data::hashCode() const {
                    return MurmurHash3_x86_32((void*)&((*data)[DATA_OFFSET]) , (int)dataSize());
                }

            }
        }
    }
}


