/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_AUTOBATCHER_H
#define HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_AUTOBATCHER_H

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/Atomic.h"
#include "hazelcast/client/flakeidgen/impl/IdBatch.h"

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                /**
                 * A utility to serve IDs from IdBatch one by one, watching for validity.
                 * It's a separate class due to testability.
                 */
                class AutoBatcher {
                public:
                    class IdBatchSupplier {
                    public:
                        virtual IdBatch newIdBatch(int32_t batchSize) = 0;

                        virtual ~IdBatchSupplier(){};
                    };

                    AutoBatcher(int32_t batchSize, int64_t validity,
                                const boost::shared_ptr<IdBatchSupplier> &batchIdSupplier);

                    /**
                     * Return next ID from current batch or get new batch from supplier if
                     * current batch is spent or expired.
                     */
                    int64_t newId();

                private:
                    class Block {
                    public:
                        Block(const IdBatch &idBatch, int64_t invalidSince);

                        /**
                         * Returns next ID or INT64_MIN, if there is none.
                         */
                        int64_t next();
                    private:
                        IdBatch idBatch;
                        int64_t invalidSince;
                        util::Atomic<int32_t> numReturned;
                    };

                    int32_t batchSize;
                    int64_t validity;
                    boost::shared_ptr<IdBatchSupplier> batchIdSupplier;

                    util::Atomic<boost::shared_ptr<Block> > block;
                    util::Mutex lock;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_FLAKEIDGEN_IMPL_AUTOBATCHER_H */
