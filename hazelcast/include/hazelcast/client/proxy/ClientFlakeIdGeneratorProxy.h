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
#ifndef HAZELCAST_CLIENT_PROXY_FLAKEIDGENERATORPROXY_H_
#define HAZELCAST_CLIENT_PROXY_FLAKEIDGENERATORPROXY_H_

#include <memory>

#include "hazelcast/client/flakeidgen/impl/AutoBatcher.h"
#include "hazelcast/client/impl/IdGeneratorInterface.h"
#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            /**
             * Client proxy implementation for a {@link PNCounter}.
             */
            class ClientFlakeIdGeneratorProxy : public impl::IdGeneratorInterface, public ProxyImpl {
            public:
                static const std::string SERVICE_NAME;

                static const int BITS_SEQUENCE = 6;
                static const int BITS_NODE_ID = 16;

                ClientFlakeIdGeneratorProxy(const std::string &objectName, spi::ClientContext *context);

                flakeidgen::impl::IdBatch newIdBatch(int32_t batchSize);

                virtual int64_t newId();

                virtual bool init(int64_t id);

            private:
                class FlakeIdBatchSupplier : public flakeidgen::impl::AutoBatcher::IdBatchSupplier {
                public:
                    FlakeIdBatchSupplier(ClientFlakeIdGeneratorProxy &proxy);

                private:
                    virtual flakeidgen::impl::IdBatch newIdBatch(int32_t batchSize);

                    ClientFlakeIdGeneratorProxy &proxy;
                };

            private:

                std::shared_ptr<flakeidgen::impl::AutoBatcher> batcher;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_FLAKEIDGENERATORPROXY_H_
