/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#pragma once
#include <vector>
#include <memory>
#include <stdint.h>
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/AtomicBoolean.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API Data {
                public:
                    // type and partition_hash are always written with BIG_ENDIAN byte-order
                    static unsigned int PARTITION_HASH_OFFSET;

                    static unsigned int TYPE_OFFSET;

                    static unsigned int DATA_OFFSET;

                    static unsigned int DATA_OVERHEAD;

                    Data();

                    Data(std::unique_ptr<std::vector<byte> > &buffer);

                    Data(std::unique_ptr<std::vector<byte> > &&buffer);

                    size_t dataSize() const;

                    size_t totalSize() const;

                    int getPartitionHash() const;

                    /**
                     * Returns the calculated hash of the data bytes.
                     * Caches the calculated value.
                     */
                    int hash() const;

                    bool hasPartitionHash() const;

                    std::vector<byte> &toByteArray() const;

                    int32_t getType() const;

                    bool operator<(const Data &rhs) const;

                private:
                    std::shared_ptr<std::vector<byte> > data;
                    int cachedHashValue;

                    inline int calculateHash() const;
                };

            }
        }
    }
}

namespace std {
    template <>
    class less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data> > {
    public:
        bool HAZELCAST_API operator() (const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
                                     const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


