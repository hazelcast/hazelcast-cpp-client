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

#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

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

                    Data(std::vector<byte> buffer);

                    size_t data_size() const;

                    size_t total_size() const;

                    int get_partition_hash() const;

                    /**
                     * Returns the calculated hash of the data bytes.
                     * Caches the calculated value.
                     */
                    int hash() const;

                    bool has_partition_hash() const;

                    const std::vector<byte> &to_byte_array() const;

                    int32_t get_type() const;

                    bool operator<(const Data &rhs) const;

                    friend bool HAZELCAST_API operator==(const Data &lhs, const Data &rhs);

                private:
                    std::vector<byte> data_;
                    int cachedHashValue_;

                    inline int calculate_hash() const;
                };

            }
        }
    }
}

namespace std {
    template <>
    class HAZELCAST_API hash<hazelcast::client::serialization::pimpl::Data> {
    public:
        std::size_t operator()(const hazelcast::client::serialization::pimpl::Data &val) const noexcept;
    };

    template<>
    class HAZELCAST_API hash<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>> {
    public:
        std::size_t
        operator()(const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &val) const noexcept;
    };

    template<>
    struct HAZELCAST_API equal_to<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>> {
        bool operator()(const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
                const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept;;
    };

    template <>
    class HAZELCAST_API less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data> > {
    public:
        bool operator() (const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
                                     const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


