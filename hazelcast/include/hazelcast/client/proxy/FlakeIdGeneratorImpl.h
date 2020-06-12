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

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API FlakeIdGeneratorImpl : public ProxyImpl {
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
                boost::future<int64_t> newId();

            protected:
                FlakeIdGeneratorImpl(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

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
                    IdBatch(int64_t base, int64_t increment, int32_t batchSize);

                    /**
                     * Returns the first ID in the set.
                     */
                    const int64_t getBase() const;

                    /**
                     * Returns increment from {@link #base()} for the next ID in the set.
                     */
                    const int64_t getIncrement() const;

                    /**
                     * Returns number of IDs in the set.
                     */
                    const int32_t getBatchSize() const;

                    class IdIterator : public std::iterator<std::input_iterator_tag, int64_t> {
                    public:
                        IdIterator();

                        IdIterator(int64_t base2, int64_t increment, int32_t remaining);

                        IdIterator &operator++();

                        bool operator==(const IdIterator &rhs) const;

                        bool operator!=(const IdIterator &rhs) const;

                        const int64_t &operator*() { return base2; }

                    private:
                        int64_t base2;
                        const int64_t increment;
                        int32_t remaining;
                    };

                    IdIterator iterator();

                    static IdIterator &end();

                private:
                    int64_t base;
                    int64_t increment;
                    int32_t batchSize;

                    static IdIterator endOfBatch;
                };

                class Block {
                public:
                    Block(IdBatch &&idBatch, std::chrono::steady_clock::duration validity);

                    /**
                     * Returns next ID or INT64_MIN, if there is none.
                     */
                    int64_t next();

                private:
                    IdBatch idBatch;
                    std::chrono::steady_clock::time_point invalidSince;
                    std::atomic<int32_t> numReturned;
                };

                boost::future<FlakeIdGeneratorImpl::IdBatch> newIdBatch(int32_t size);

                int64_t newIdInternal();

                int32_t batchSize;
                std::chrono::steady_clock::duration validity;
                boost::atomic_shared_ptr<Block> block;
                std::mutex lock;
            };

        }
    }
}

