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
#include <hazelcast/util/Bits.h>

using namespace hazelcast::client::util;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                // type and partition_hash are always written with BIG_ENDIAN byte-order
                unsigned int Data::TYPE_OFFSET = 0;
                // will use a byte to store partition_hash bit
                unsigned int Data::PARTITION_HASH_BIT_OFFSET = 4;
                unsigned int Data::DATA_OFFSET = 5;

                // array (12: array header, 4: length)
                unsigned int Data::ARRAY_HEADER_SIZE_IN_BYTES = 16;

                Data::Data()
                : data(new BufferType){
                }

                Data::Data(BufferType_ptr buffer) : data(buffer) {
                    if (buffer.get() != 0 && buffer->size() > 0 && buffer->size() < DATA_OFFSET) {
                        char msg[100 + 1];
                        snprintf(msg, 100, "Provided buffer should be either empty or "
                                "should contain more than %d bytes! Provided buffer size:%ld", DATA_OFFSET, buffer->size());
                        throw hazelcast::client::exception::IllegalArgumentException("Data::setBuffer", msg);
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


                unsigned long Data::dataSize() const {
                    return std::max(totalSize() - DATA_OFFSET, (unsigned long)0);
                }

                unsigned long Data::totalSize() const {
                    return data.get() != 0 ? (int)data->size() : 0;
                }

                int Data::getPartitionHash() const {
                    if (hasPartitionHash()) {
                        return Bits::readIntB(data.get(), data->size() - Bits::INT_SIZE_IN_BYTES);
                    }
                    return hashCode();
                }

                bool Data::hasPartitionHash() const {
                    return totalSize() != 0 && (*data)[PARTITION_HASH_BIT_OFFSET] != 0;
                }

                Data::BufferType &Data::toByteArray() const {
                    return *data;
                }

                int Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return Bits::readIntB(data.get(), TYPE_OFFSET);
                }

                unsigned long Data::getHeapCost() const {
                    // reference (assuming compressed oops)
                    int objectRef = Bits::INT_SIZE_IN_BYTES;
                    return objectRef + (data.get() != 0 ? ARRAY_HEADER_SIZE_IN_BYTES + data->size() : 0);
                }

                int Data::hashCode() const {
                    return hazelcast::util::MurmurHash3_x86_32((void*)&((*data)[0]) , (int)data->size());
                }

                bool Data::isPortable() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE == getType();
                }


            }
        }
    }
}


