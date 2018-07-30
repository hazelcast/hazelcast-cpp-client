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
#include "hazelcast/client/flakeidgen/impl/FlakeIdGeneratorProxyFactory.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace flakeidgen {
            namespace impl {
                FlakeIdGeneratorProxyFactory::FlakeIdGeneratorProxyFactory(spi::ClientContext *clientContext) : clientContext(
                        clientContext) {}

                boost::shared_ptr<spi::ClientProxy> FlakeIdGeneratorProxyFactory::create(const std::string &id) {
                    return boost::shared_ptr<spi::ClientProxy>(
                            new proxy::ClientFlakeIdGeneratorProxy(proxy::ClientFlakeIdGeneratorProxy::SERVICE_NAME, id,
                                                            clientContext));
                }
            }
        }
    }
}
