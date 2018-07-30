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
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientFlakeIdGeneratorProxy::SERVICE_NAME = "hz:impl:flakeIdGeneratorService";

            ClientFlakeIdGeneratorProxy::ClientFlakeIdGeneratorProxy(const std::string &serviceName,
                                                                     const std::string &objectName,
                                                                     spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context) {
                boost::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = getContext().getClientConfig().findFlakeIdGeneratorConfig(getName());
                batcher.reset(
                        new flakeidgen::impl::AutoBatcher(config->getPrefetchCount(), config->getPrefetchValidityMillis(),
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

            ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::FlakeIdBatchSupplier(ClientFlakeIdGeneratorProxy &proxy)
                    : proxy(proxy) {}

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::newIdBatch(int32_t batchSize) {
                return proxy.newIdBatch(batchSize);
            }
        }
    }
}
