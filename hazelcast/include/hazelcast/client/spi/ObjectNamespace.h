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

#ifndef HAZELCAST_CLIENT_SPI_OBJECTNAMESPACE_H_
#define HAZELCAST_CLIENT_SPI_OBJECTNAMESPACE_H_

#include <string>
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            /**
             * An ObjectNamespace that makes identification of object within a service possible.
             */
            class HAZELCAST_API ObjectNamespace {
            public:
                virtual ~ObjectNamespace() { }

                /**
                 * Gets the service name.
                 *
                 * @return the service name
                 */
                virtual const std::string &getServiceName() const = 0;

                /**
                 * Gets the object name within the service.
                 *
                 * @return the object name within the service
                 */
                virtual const std::string &getObjectName() const = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_OBJECTNAMESPACE_H_

