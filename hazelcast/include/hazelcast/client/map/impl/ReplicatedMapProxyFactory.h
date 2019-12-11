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
#ifndef HAZELCAST_CLIENT_MAP_IMPL_REPLICATEDMAPPROXYFACTORY_H_
#define HAZELCAST_CLIENT_MAP_IMPL_REPLICATEDMAPPROXYFACTORY_H_

#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/proxy/ClientReplicatedMapProxy.h"

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                template<typename K, typename V>
                class ReplicatedMapProxyFactory : public spi::ClientProxyFactory {
                public:
                    ReplicatedMapProxyFactory(spi::ClientContext *context) : clientContext(context) {
                    }

                    //@Override
                    boost::shared_ptr<spi::ClientProxy> create(const std::string &name) {
                        return boost::shared_ptr<spi::ClientProxy>(
                                new proxy::ClientReplicatedMapProxy<K, V>(name, clientContext));
                    }

                private:
                    spi::ClientContext *clientContext;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_MAP_IMPL_REPLICATEDMAPPROXYFACTORY_H_ */

