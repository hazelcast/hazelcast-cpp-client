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
#ifndef HAZELCAST_CLIENT_SOCKET_H_
#define HAZELCAST_CLIENT_SOCKET_H_

#include "hazelcast/client/Address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        /**
         * Sockets wrapper interface class.
         */
        class HAZELCAST_API Socket {
        public:
            /**
             * Destructor
             */
            virtual ~Socket();
            /**
             * connects to given address in constructor.
             * @param timeoutInMillis if not connected within timeout, it will return errorCode
             * @return zero if error. -1 otherwise.
             */
            virtual int connect(int timeoutInMillis) = 0;

            /**
             * @param buffer
             * @param len length of the buffer
             * @return number of bytes send
             * @throw IOException in failure.
             */
            virtual int send(const void *buffer, int len) const = 0;

            /**
             * @param buffer
             * @param len  length of the buffer to be received.
             * @param flag bsd sockets options flag.
             * @return number of bytes received.
             * @throw IOException in failure.
             */
            virtual int receive(void *buffer, int len, int flag = 0) const = 0;

            /**
             * @param buffer
             * @param len  length of the buffer to be received.
             * @return number of bytes received.
             * @throw IOException in failure.
             */
            virtual int receiveBlocking(void *buffer, int len, int flag = 0) const {return receive(buffer, len, flag);}

            /**
             * return socketId
             */
            virtual int getSocketId() const = 0;

            /**
             * @param address remote endpoint address.
             */
            virtual void setRemoteEndpoint(const client::Address &address) = 0;

            /**
             * @return remoteEndpoint
             */
            virtual const client::Address &getRemoteEndpoint() const = 0;

            /**
             * closes the socket. Automatically called in destructor.
             * Second call to this function is no op.
             */
            virtual void close() = 0;

            virtual client::Address getAddress() const = 0;

            virtual void setBlocking(bool blocking) = 0;
        };

        /**
         * Socket Ptr compare method. Compares based on socket id.
         */
        struct HAZELCAST_API socketPtrComp {
            /**
             * Functor.
             */
            bool operator ()(Socket const *const &lhs, Socket const *const &rhs) const;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_SOCKET_H_ */
