/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
             * Client proxy implementation for a {@link PNCounter}.
             */
            class ClientIdGeneratorProxy : public impl::IdGeneratorInterface, public proxy::ProxyImpl {
            public:
                static const std::string SERVICE_NAME;

                enum {
                    BLOCK_SIZE = 1000
                };

                ClientIdGeneratorProxy(const std::string &instanceName, spi::ClientContext *context);

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

            private:
                IAtomicLong atomicLong;
                boost::shared_ptr<util::Atomic<int64_t> > local;
                boost::shared_ptr<util::Atomic<int32_t> > residue;
                boost::shared_ptr<util::Mutex> localLock;

                void onDestroy();
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_IDGENERATORPROXY_H_
