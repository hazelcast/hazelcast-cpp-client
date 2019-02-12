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

#include "hazelcast/client/proxy/ClientFlakeIdGeneratorProxy.h"
#include "hazelcast/client/protocol/codec/FlakeIdGeneratorNewIdBatchCodec.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientFlakeIdGeneratorProxy::SERVICE_NAME = "hz:impl:flakeIdGeneratorService";

            ClientFlakeIdGeneratorProxy::ClientFlakeIdGeneratorProxy(const std::string &objectName,
                                                                     spi::ClientContext *context)
                    : ProxyImpl(SERVICE_NAME, objectName, context) {
                boost::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = getContext().getClientConfig().findFlakeIdGeneratorConfig(
                        getName());
                batcher.reset(
                        new flakeidgen::impl::AutoBatcher(config->getPrefetchCount(),
                                                          config->getPrefetchValidityMillis(),
                                                          boost::shared_ptr<flakeidgen::impl::AutoBatcher::IdBatchSupplier>(
                                                                  new FlakeIdBatchSupplier(*this))));
            }

            int64_t ClientFlakeIdGeneratorProxy::newId() {
                return batcher->newId();
            }

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::newIdBatch(int32_t batchSize) {
                std::auto_ptr<protocol::ClientMessage> requestMsg = protocol::codec::FlakeIdGeneratorNewIdBatchCodec::encodeRequest(
                        getName(), batchSize);
                boost::shared_ptr<protocol::ClientMessage> responseMsg = spi::impl::ClientInvocation::create(
                        getContext(), requestMsg, getName())->invoke()->get();
                protocol::codec::FlakeIdGeneratorNewIdBatchCodec::ResponseParameters response =
                        protocol::codec::FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::decode(*responseMsg);
                return flakeidgen::impl::IdBatch(response.base, response.increment, response.batchSize);
            }

            bool ClientFlakeIdGeneratorProxy::init(int64_t id) {
                // Add 1 hour worth of IDs as a reserve: due to long batch validity some clients might be still getting
                // older IDs. 1 hour is just a safe enough value, not a real guarantee: some clients might have longer
                // validity.
                // The init method should normally be called before any client generated IDs: in this case no reserve is
                // needed, so we don't want to increase the reserve excessively.
                int64_t reserve =
                        (int64_t) (3600 * 1000) /* 1 HOUR in milliseconds */ << (BITS_NODE_ID + BITS_SEQUENCE);
                return newId() >= id + reserve;
            }

            ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::FlakeIdBatchSupplier(ClientFlakeIdGeneratorProxy &proxy)
                    : proxy(proxy) {}

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::newIdBatch(int32_t batchSize) {
                return proxy.newIdBatch(batchSize);
            }
        }
    }
}
