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
#ifndef HAZELCAST_CLIENT_MAP_IMPL_MIXEDMAPPROXYFACTORY_H_
#define HAZELCAST_CLIENT_MAP_IMPL_MIXEDMAPPROXYFACTORY_H_

#include "hazelcast/client/mixedtype/NearCachedClientMapProxy.h"
#include "hazelcast/client/mixedtype/ClientMapProxy.h"
#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/map/NearCachedClientMapProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                class HAZELCAST_API MapMixedTypeProxyFactory : public spi::ClientProxyFactory {
                public:
                    MapMixedTypeProxyFactory(spi::ClientContext *context) : clientContext(context) {
                    }

                    //@Override
                    std::shared_ptr<spi::ClientProxy> create(const std::string &name) {
                        const std::shared_ptr<mixedtype::config::MixedNearCacheConfig> nearCacheConfig =
                                clientContext->getClientConfig().getMixedNearCacheConfig(name);
                        spi::ClientProxy *proxy;
                        if (nearCacheConfig != NULL) {
                            //TODO checkNearCacheConfig(nearCacheConfig, true);
                            proxy = new mixedtype::NearCachedClientMapProxy(name, clientContext, *nearCacheConfig);
                        } else {
                            proxy = new mixedtype::ClientMapProxy(name, clientContext);
                        }

                        return std::shared_ptr<spi::ClientProxy>(proxy);
                    }
                private:
                    spi::ClientContext *clientContext;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_IMPL_MIXEDMAPPROXYFACTORY_H_ */

