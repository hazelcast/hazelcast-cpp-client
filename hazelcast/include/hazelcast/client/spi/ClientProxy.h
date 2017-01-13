/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 30 Nov 2016.

#ifndef HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_
#define HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class IDistributedObject;

        namespace spi {
            /**
             * Base Interface for client proxies.
             *
             */
            class HAZELCAST_API ClientProxy {
            public:
                virtual ~ClientProxy() {
                }

                /**
                 * Called when proxy is created.
                 * Overriding implementations can add initialization specific logic into this method
                 * like registering a listener, creating a cleanup task etc.
                 */
                virtual void onInitialize() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_CLIENTPROXY_H_

