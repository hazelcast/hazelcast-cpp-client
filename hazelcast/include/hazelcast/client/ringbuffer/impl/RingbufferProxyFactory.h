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
#pragma once
#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/proxy/ClientRingbufferProxy.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }
        namespace ringbuffer {
            namespace impl {
                template<typename T>
                class RingbufferProxyFactory : public spi::ClientProxyFactory {
                public:
                    RingbufferProxyFactory(spi::ClientContext *clientContext) : clientContext(
                            clientContext) {}

                    virtual std::shared_ptr<spi::ClientProxy> create(const std::string &id) {
                        return std::shared_ptr<spi::ClientProxy>(
                                new proxy::ClientRingbufferProxy<T>(id, clientContext));
                    }

                private:
                    spi::ClientContext *clientContext;
                };
            }
        }
    }
}

