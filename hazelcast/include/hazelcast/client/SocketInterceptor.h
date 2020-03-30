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
//
// Created by sancar koyunlu on 8/2/13.

#ifndef HAZELCAST_SocketInterceptor
#define HAZELCAST_SocketInterceptor

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class Socket;

        /**
         * Base class for socketInterceptor classes to inherit from.
         *
         * @see ClientConfig#setSocketInterceptor(SocketInterceptor *socketInterceptor);
         */
        class HAZELCAST_API SocketInterceptor {
        public:
            /**
             * Will be called with the Socket, each time client creates a connection to any Member.
             */
            virtual void onConnect(const Socket &connectedSocket) = 0;

            virtual ~SocketInterceptor();
        };
    }
}

#endif //HAZELCAST_SocketInterceptor

