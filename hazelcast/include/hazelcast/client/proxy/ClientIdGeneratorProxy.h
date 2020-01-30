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
#ifndef HAZELCAST_CLIENT_PROXY_IDGENERATORPROXY_H_
#define HAZELCAST_CLIENT_PROXY_IDGENERATORPROXY_H_

#include "hazelcast/client/impl/IdGeneratorInterface.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/IAtomicLong.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            /**
             * Client proxy implementation for a {@link IdGeneratorInterface}.
             */
            class ClientIdGeneratorProxy : public impl::IdGeneratorInterface, public proxy::ProxyImpl {
            public:
                static const std::string SERVICE_NAME;

                static const std::string ATOMIC_LONG_NAME;

                enum {
                    BLOCK_SIZE = 10000
                };

                ClientIdGeneratorProxy(const std::string &instanceName, spi::ClientContext *context,
                                       const IAtomicLong &atomicLong);

                /**
                 * Try to initialize this IdGenerator instance with given id
                 *
                 * @return true if initialization success
                 */
                virtual bool init(int64_t id);

                /**
                 * Generates and returns cluster-wide unique id.
                 * Generated ids are guaranteed to be unique for the entire cluster
                 * as long as the cluster is live. If the cluster restarts then
                 * id generation will start from 0.
                 *
                 * @return cluster-wide new unique id
                 */
                int64_t newId();

                void destroy();
            private:
                IAtomicLong atomicLong;
                std::shared_ptr<std::atomic<int64_t> > local;
                std::shared_ptr<std::atomic<int32_t> > residue;
                std::shared_ptr<util::Mutex> localLock;

            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_IDGENERATORPROXY_H_
