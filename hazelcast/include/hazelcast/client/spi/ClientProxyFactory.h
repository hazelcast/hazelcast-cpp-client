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
//
// Created by ihsan demir on 30 Nov 2016.

#ifndef HAZELCAST_CLIENT_SPI_CLIENTPROXYFACTORY_H_
#define HAZELCAST_CLIENT_SPI_CLIENTPROXYFACTORY_H_

#include <string>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientProxy;

            /**
             * Factory interface creating client proxies.
             *
             * Instances implementing this interface can be registered with the {@link ProxyManager} for instantiation upon request.
             */
            class HAZELCAST_API ClientProxyFactory {
            public:
                virtual ~ClientProxyFactory() { }

                /**
                 * Creates a new client proxy with the given id.
                 *
                 * @param id the ID of the client proxy
                 * @return the client proxy
                 */
                virtual boost::shared_ptr<ClientProxy> create(const std::string &id) = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_CLIENTPROXYFACTORY_H_

