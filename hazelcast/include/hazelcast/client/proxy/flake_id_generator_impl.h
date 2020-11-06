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

#include <memory>
#include <atomic>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/client/proxy/ProxyImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API flake_id_generator_impl : public ProxyImpl {
            public:
                /**
                 * Generates and returns a cluster-wide unique ID.
                 * <p>
                 * Operation on member is always local, if the member has valid node ID, otherwise it's remote. On
                 * client, this method goes to a random member and gets a batch of IDs, which will then be returned
                 * locally for limited time. The pre-fetch size and the validity time can be configured for
                 * each client and member, see {@code ClientConfig.addFlakeIdGeneratorConfig()} for client config.
                 * <p>
                 * <b>Note:</b> Values returned from this method may be not strictly ordered.
                 *
                 * @return new cluster-wide unique ID
                 *
                 * @throws NodeIdOutOfRangeException if node ID for all members in the cluster is out of valid range.
                 *      See "Node ID overflow" in {@link FlakeIdGenerator class documentation} for more details.
                 */
                boost::future<int64_t> new_id();

            protected:
                flake_id_generator_impl(const std::string &service_name, const std::string &object_name, spi::ClientContext *context);

            private:
                /**
                * Set of IDs returned from {@link FlakeIdGenerator}.
                * <p>
                * IDs can be iterated using a foreach loop:
                * <pre>{@code
                *    IdBatch idBatch = myFlakeIdGenerator.newIdBatch(100);
                *    for (Long id : idBatch) {
                *        // ... use the id
                *    }
                * }</pre>
                * <p>
                * Object is immutable.
                */
                class IdBatch {
                public:
                    IdBatch(int64_t base, int64_t increment, int32_t batch_size);

                    /**
                     * Returns the first ID in the set.
                     */
                    const int64_t get_base() const;

                    /**
                     * Returns increment from {@link #base()} for the next ID in the set.
                     */
                    const int64_t get_increment() const;

                    /**
                     * Returns number of IDs in the set.
                     */
                    const int32_t get_batch_size() const;

                    class IdIterator : public std::iterator<std::input_iterator_tag, int64_t> {
                    public:
                        IdIterator();

                        IdIterator(int64_t base2, int64_t increment, int32_t remaining);

                        IdIterator &operator++();

                        bool operator==(const IdIterator &rhs) const;

                        bool operator!=(const IdIterator &rhs) const;

                        const int64_t &operator*() { return base2_; }

                    private:
                        int64_t base2_;
                        const int64_t increment_;
                        int32_t remaining_;
                    };

                    IdIterator iterator();

                    static IdIterator &end();

                private:
                    int64_t base_;
                    int64_t increment_;
                    int32_t batch_size_;

                    static IdIterator endOfBatch;
                };

                class Block {
                public:
                    Block(IdBatch &&id_batch, std::chrono::milliseconds validity);

                    /**
                     * Returns next ID or INT64_MIN, if there is none.
                     */
                    int64_t next();

                private:
                    IdBatch id_batch_;
                    std::chrono::steady_clock::time_point invalid_since_;
                    std::atomic<int32_t> num_returned_;
                };

                boost::future<flake_id_generator_impl::IdBatch> new_id_batch(int32_t size);

                int64_t new_id_internal();

                int32_t batch_size_;
                std::chrono::milliseconds validity_;
                boost::atomic_shared_ptr<Block> block_;
                std::mutex lock_;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

