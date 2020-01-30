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
//
// Created by sancar koyunlu on 30/12/13.
//

#include <string.h>

#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace util {

        ServerSocket::ServerSocket(int port) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
            if(n == -1) 
				throw client::exception::IOException("Socket::Socket ", "WSAStartup error");
			#endif
            struct addrinfo hints;
            struct addrinfo *serverInfo;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
            ::getaddrinfo(NULL, IOUtil::to_string(port).c_str(), &hints, &serverInfo);
            socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
            isOpen.store(true);
			if(serverInfo->ai_family == AF_INET){
				ipv4 = true;
			}else if(serverInfo->ai_family == AF_INET6){
				ipv4 = false;
			}else{
				throw client::exception::IOException("ServerSocket(int)","unsupported ip protocol");
			}
            ::bind(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen);
            ::listen(socketId, 10);
            ::freeaddrinfo(serverInfo);

        }


        ServerSocket::~ServerSocket() {
            close();
        }

		bool ServerSocket::isIpv4() const{
			return ipv4;
		}

        void ServerSocket::close() {
            bool expected = true;
            if (isOpen.compare_exchange_strong(expected, false)) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
				::shutdown(socketId, SD_RECEIVE);
				char buffer[1];
                ::recv(socketId, buffer, 1, MSG_WAITALL);
				WSACleanup();
                closesocket(socketId);
                #else
				::shutdown(socketId, SHUT_RD);
				char buffer[1];
                ::recv(socketId, buffer, 1, MSG_WAITALL);
                ::close(socketId);
                #endif
            }

        }

        int ServerSocket::getPort() const {
			if(ipv4){
				struct sockaddr_in sin;
				socklen_t len = sizeof(sin);
				if (getsockname(socketId, (struct sockaddr *)&sin, &len) == 0 && sin.sin_family == AF_INET){
					return ntohs(sin.sin_port);	
				}
				throw client::exception::IOException("ServerSocket::getPort()", "getsockname");
			}
            
			struct sockaddr_in6 sin6;
			socklen_t len = sizeof(sin6);
			if (getsockname(socketId, (struct sockaddr *)&sin6, &len) == 0 && sin6.sin6_family == AF_INET6){
				return ntohs(sin6.sin6_port);	
			}
            throw client::exception::IOException("ServerSocket::getPort()", "getsockname");
        }

        client::Socket *ServerSocket::accept() {
            struct sockaddr_storage their_address;
            socklen_t address_size = sizeof their_address;
            int sId = ::accept(socketId, (struct sockaddr *) &their_address, &address_size);

            if (sId == -1) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int error =   WSAGetLastError();
                #else
                int error = errno;
                #endif
                throw client::exception::IOException("Socket::accept", strerror(error));
            }
            return new client::internal::socket::TcpSocket(sId);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
