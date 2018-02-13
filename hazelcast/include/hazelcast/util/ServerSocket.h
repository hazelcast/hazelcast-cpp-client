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
// Created by sancar koyunlu on 30/12/13.
//


#ifndef HAZELCAST_ServerSocket
#define HAZELCAST_ServerSocket

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicBoolean.h"

namespace hazelcast {
    namespace client{
        class Socket;
    }
    namespace util {
        class ServerSocket {
        public:
            ServerSocket(int port = 0);

            ~ServerSocket();

            client::Socket *accept();

            int getPort() const;

            void close();

			bool isIpv4() const;

        private:
            int socketId;
			bool ipv4;
            util::AtomicBoolean isOpen;

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			WSADATA wsa_data;
			#endif

        };
    }
}


#endif //HAZELCAST_ServerSocket

