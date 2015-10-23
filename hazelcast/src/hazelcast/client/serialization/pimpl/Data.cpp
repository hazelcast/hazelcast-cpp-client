/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

                unsigned int Data::DATA_OFFSET = Bits::INT_SIZE_IN_BYTES;

                //first 4 byte is type id + last 4 byte is partition hash code
                unsigned int Data::DATA_OVERHEAD = DATA_OFFSET + Bits::INT_SIZE_IN_BYTES;

                Data::Data()
                : data(NULL){
                }

                Data::Data(std::auto_ptr<std::vector<byte> > buffer) : data(buffer) {
                    if (data.get() != 0 && data->size() > 0 && data->size() < DATA_OVERHEAD) {
                        char msg[100];
                        sprintf(msg, "Provided buffer should be either empty or "
                                "should contain more than %u bytes! Provided buffer size:%lu", DATA_OVERHEAD, (unsigned long)data->size());
                        throw exception::IllegalArgumentException("Data::setBuffer", msg);
                    }
                }

                Data::Data(const Data& rhs)
                : data(rhs.data) {

                }

                Data& Data::operator=(const Data& rhs) {
                    data = rhs.data;
                    return (*this);
                }


                size_t Data::dataSize() const {
                    return (size_t)std::max<int>((int)totalSize() - (int)DATA_OVERHEAD, 0);
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
                    size_t length = data->size();
                    return data.get() != NULL && length >= DATA_OVERHEAD &&
                            *reinterpret_cast<int *>(&((*data)[length - Bits::INT_SIZE_IN_BYTES])) != 0;
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


