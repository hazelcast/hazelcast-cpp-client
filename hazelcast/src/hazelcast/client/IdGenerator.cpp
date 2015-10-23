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
#include "hazelcast/client/IdGenerator.h"

namespace hazelcast {
    namespace client {

        IdGenerator::IdGenerator(const std::string& instanceName, spi::ClientContext *context)
        : proxy::ProxyImpl("idGeneratorService", instanceName, context)
        , atomicLong("hz:atomic:idGenerator:" + instanceName, context)
        , local(new util::AtomicInt(-1))
        , residue(new util::AtomicInt(BLOCK_SIZE))
        , localLock(new util::Mutex) {

        }


        bool IdGenerator::init(long id) {
            if (id <= 0) {
                return false;
            }
            long step = (id / BLOCK_SIZE);

            util::LockGuard lg(*localLock);
            bool init = atomicLong.compareAndSet(0, step + 1);
            if (init) {
                *local = step;
                *residue = (id % BLOCK_SIZE) + 1;
            }
            return init;
        }

        long IdGenerator::newId() {
            int value = (*residue)++;
            if (value >= BLOCK_SIZE) {
                util::LockGuard lg(*localLock);
                value = *residue;
                if (value >= BLOCK_SIZE) {
                    *local = atomicLong.getAndIncrement();
                    *residue = 0;
                }
                return newId();

            }
            return int(*local) * BLOCK_SIZE + value;
        }

        void IdGenerator::onDestroy() {
            atomicLong.onDestroy();
        }

    }
}
