/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_
#define HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_

#include <stdexcept>
#include <climits>
#include <hazelcast/client/config/NearCacheConfig.h>
#include "hazelcast/client/map/ClientMapProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            /**
             * A Client-side {@code IMap} implementation which is fronted by a near-cache.
             *
             * @param <K> the key type for this {@code IMap} proxy.
             * @param <V> the value type for this {@code IMap} proxy.
             */
            template<typename K, typename V>
            class NearCachedClientMapProxy : public ClientMapProxy<K, V> {
            public:
                NearCachedClientMapProxy(const std::string &instanceName, spi::ClientContext *context,
                                         const config::NearCacheConfig *nearcacheConfig)
                        : ClientMapProxy<K, V>(instanceName, context) {
                    // TODO: implement it
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_ */

