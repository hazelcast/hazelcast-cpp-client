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
#ifndef HAZELCAST_CLIENT_CRDT_PNCOUNTER_IMPL_PNCOUNTERPROXYFACTORY_H_
#define HAZELCAST_CLIENT_CRDT_PNCOUNTER_IMPL_PNCOUNTERPROXYFACTORY_H_

#include "hazelcast/client/spi/ClientProxyFactory.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }
        namespace crdt {
            namespace pncounter {
                namespace impl {
                    class PNCounterProxyFactory : public spi::ClientProxyFactory {
                    public:
                        PNCounterProxyFactory(spi::ClientContext *clientContext);

                        virtual std::shared_ptr<spi::ClientProxy> create(const std::string &id);

                    private:
                        spi::ClientContext *clientContext;
                    };
                }
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_CRDT_PNCOUNTER_IMPL_PNCOUNTERPROXYFACTORY_H_ */
