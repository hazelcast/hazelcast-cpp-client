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
#include "hazelcast/util/Util.h"

#include <algorithm>


using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                //first 4 byte is partition hash code and next last 4 byte is type id
                unsigned int Data::PARTITION_HASH_OFFSET = 0;

                unsigned int Data::TYPE_OFFSET = Data::PARTITION_HASH_OFFSET + Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OFFSET = Data::TYPE_OFFSET + Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OVERHEAD = Data::DATA_OFFSET;

                Data::Data()
                : data(NULL), calculatedHash(0), calculatedHashExist(false) {
                }

                Data::Data(std::auto_ptr<std::vector<byte> > buffer) : data(buffer), calculatedHash(-1),
                                                                       calculatedHashExist(false) {
                    if (data.get() != 0 && data->size() > 0 && data->size() < Data::DATA_OVERHEAD) {
                        char msg[100];
                        util::snprintf(msg, 100, "Provided buffer should be either empty or "
                                "should contain more than %u bytes! Provided buffer size:%lu", Data::DATA_OVERHEAD, (unsigned long)data->size());
                        throw exception::IllegalArgumentException("Data::setBuffer", msg);
                    }
                }

                Data::Data(const Data& rhs)
                : data(rhs.data) {
                    int hash = rhs.calculatedHash;
                    calculatedHash = hash;
                    bool isCalculated = rhs.calculatedHashExist;
                    calculatedHashExist = isCalculated;
                }

                Data& Data::operator=(const Data& rhs) {
                    data = rhs.data;
                    int hash = rhs.calculatedHash;
                    calculatedHash = hash;
                    bool isCalculated = rhs.calculatedHashExist;
                    calculatedHashExist = isCalculated;
                    return (*this);
                }

                size_t Data::dataSize() const {
                    return (size_t)std::max<int>((int)totalSize() - (int)Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data.get() != 0 ? data->size() : 0;
                }

                int Data::getPartitionHash() const {
                    if (hasPartitionHash()) {
                        return Bits::readIntB(*data, Data::PARTITION_HASH_OFFSET);
                    }
                    return hashCode();
                }

                bool Data::hasPartitionHash() const {
                    size_t length = data->size();
                    return data.get() != NULL && length >= Data::DATA_OVERHEAD &&
                            *reinterpret_cast<int *>(&((*data)[PARTITION_HASH_OFFSET])) != 0;
                }

                std::vector<byte>  &Data::toByteArray() const {
                    return *data;
                }

                int Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return Bits::readIntB(*data, Data::TYPE_OFFSET);
                }

                int Data::hashCode() const {
                    if (!calculatedHashExist) {
                        calculatedHash = MurmurHash3_x86_32((void*)&((*data)[Data::DATA_OFFSET]) , (int)dataSize());
                        calculatedHashExist = true;
                    }
                    return calculatedHash;
                }
            }
        }
    }
}

namespace boost {
    /**
     * Template specialization for the less operator comparing two shared_ptr Data.
     */
    template<>
    bool operator <(const boost::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs, const boost::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) {
        const hazelcast::client::serialization::pimpl::Data *leftPtr = lhs.get();
        const hazelcast::client::serialization::pimpl::Data *rightPtr = rhs.get();
        if (leftPtr == rightPtr) {
            return false;
        }

        if (leftPtr == NULL) {
            return true;
        }

        if (rightPtr == NULL) {
            return false;
        }

        return lhs->getPartitionHash() < rhs->getPartitionHash();
    }
}


