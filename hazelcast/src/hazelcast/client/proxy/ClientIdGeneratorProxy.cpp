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
#include "hazelcast/client/proxy/ClientIdGeneratorProxy.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientIdGeneratorProxy::SERVICE_NAME = "hz:impl:idGeneratorService";
            const std::string ClientIdGeneratorProxy::ATOMIC_LONG_NAME = "hz:atomic:idGenerator:";

            ClientIdGeneratorProxy::ClientIdGeneratorProxy(const std::string &instanceName, spi::ClientContext *context,
                                                           const IAtomicLong &atomicLong)
                    : proxy::ProxyImpl(ClientIdGeneratorProxy::SERVICE_NAME, instanceName, context),
                      atomicLong(atomicLong), local(new std::atomic<int64_t>(-1)),
                      residue(new std::atomic<int32_t>(BLOCK_SIZE)), localLock(new util::Mutex) {
                this->atomicLong.get();
            }

            bool ClientIdGeneratorProxy::init(int64_t id) {
                if (id < 0) {
                    return false;
                }
                int64_t step = (id / BLOCK_SIZE);

                util::LockGuard lg(*localLock);
                bool init = atomicLong.compareAndSet(0, step + 1);
                if (init) {
                    local->store(step);
                    residue->store((int32_t) (id % BLOCK_SIZE) + 1);
                }
                return init;
            }

            int64_t ClientIdGeneratorProxy::newId() {
                int64_t block = local->load();
                int32_t value = (*residue)++;

                if (local->load() != block) {
                    return newId();
                }

                if (value < BLOCK_SIZE) {
                    return block * BLOCK_SIZE + value;
                }

                {
                    util::LockGuard lg(*localLock);
                    value = *residue;
                    if (value >= BLOCK_SIZE) {
                        *local = atomicLong.getAndIncrement();
                        *residue = 0;
                    }
                }

                return newId();
            }

            void ClientIdGeneratorProxy::destroy() {
                util::LockGuard lg(*localLock);
                atomicLong.destroy();
                *local = -1;
                *residue = BLOCK_SIZE;
            }
        }
    }
}
