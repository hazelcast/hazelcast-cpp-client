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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/flakeidgen/impl/AutoBatcher.h"

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {

                AutoBatcher::AutoBatcher(int32_t batchSize, int64_t validity,
                                         const boost::shared_ptr<AutoBatcher::IdBatchSupplier> &batchIdSupplier)
                        : batchSize(batchSize), validity(validity), batchIdSupplier(batchIdSupplier),
                          block(boost::shared_ptr<Block>(new Block(IdBatch(0, 0, 0), 0))) {}

                int64_t AutoBatcher::newId() {
                    for (;;) {
                        boost::shared_ptr<Block> block = this->block;
                        int64_t res = block->next();
                        if (res != INT64_MIN) {
                            return res;
                        }

                        {
                            util::LockGuard guard(lock);
                            if (block != this->block.get()) {
                                // new block was assigned in the meantime
                                continue;
                            }
                            this->block = boost::shared_ptr<Block>(
                                    new Block(batchIdSupplier->newIdBatch(batchSize), validity));
                        }
                    }
                }

                AutoBatcher::Block::Block(const IdBatch &idBatch, int64_t validity) : idBatch(idBatch), numReturned(0) {
                    invalidSince = validity > 0 ? util::currentTimeMillis() + validity : INT64_MAX;
                }

                int64_t AutoBatcher::Block::next() {
                    if (invalidSince <= util::currentTimeMillis()) {
                        return INT64_MIN;
                    }
                    int32_t index;
                    do {
                        index = numReturned;
                        if (index == idBatch.getBatchSize()) {
                            return INT64_MIN;
                        }
                    } while (!numReturned.compareAndSet(index, index + 1));

                    return idBatch.getBase() + index * idBatch.getIncrement();
                }
            }
        }
    }
}
