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
#ifndef HAZELCAST_CLIENT_CONFIG_SOCKETOPTIONS_H_
#define HAZELCAST_CLIENT_CONFIG_SOCKETOPTIONS_H_

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * TCP Socket options
             */
            class HAZELCAST_API SocketOptions {
            public:
                /**
                 * constant for kilobyte
                 */
                static const int KILO_BYTE = 1024;

                /**
                 * default buffer size of Bytes
                 */
                static const int DEFAULT_BUFFER_SIZE_BYTE = 128 * KILO_BYTE;

                SocketOptions();

                /**
                 * TCP_NODELAY socket option
                 *
                 * @return true if enabled
                 */
                bool isTcpNoDelay() const;

                /**
                 * Enable/disable TCP_NODELAY socket option.
                 *
                 * @param tcpNoDelay
                 */
                SocketOptions &setTcpNoDelay(bool tcpNoDelay);

                /**
                 * SO_KEEPALIVE socket option
                 *
                 * @return true if enabled
                 */
                bool isKeepAlive() const;

                /**
                 * Enable/disable SO_KEEPALIVE socket option.
                 *
                 * @param keepAlive enabled if true
                 * @return SocketOptions configured
                 */
                SocketOptions &setKeepAlive(bool keepAlive);

                /**
                 * SO_REUSEADDR socket option.
                 *
                 * @return true if enabled
                 */
                bool isReuseAddress() const;

                /**
                 * Enable/disable the SO_REUSEADDR socket option.
                 *
                 * @param reuseAddress enabled if true
                 * @return SocketOptions configured
                 */
                SocketOptions &setReuseAddress(bool reuseAddress);

                /**
                 * Gets SO_LINGER with the specified linger time in seconds
                 * @return lingerSeconds value in seconds
                 */
                int getLingerSeconds() const;

                /**
                 * Enable/disable SO_LINGER with the specified linger time in seconds
                 *
                 * if set to a value of 0 or less then it is disabled.
                 *
                 * Default value is 3.
                 *
                 * @param lingerSeconds value in seconds
                 * @return SocketOptions configured
                 */
                SocketOptions &setLingerSeconds(int lingerSeconds);

                /**
                 * If set to 0 or less, then it is not set on the socket.
                 *
                 * The default value is DEFAULT_BUFFER_SIZE_BYTE
                 *
                 * Gets the SO_SNDBUF and SO_RCVBUF options value in bytes
                 * @return bufferSize Number of bytes
                 */
                int getBufferSizeInBytes() const;

                /**
                 * If set to 0 or less, then it is not set on the socket.
                 *
                 * The default value is DEFAULT_BUFFER_SIZE_BYTE
                 *
                 * Sets the SO_SNDBUF and SO_RCVBUF options to the specified value in bytes
                 *
                 * @param bufferSize Number of bytes
                 * @return SocketOptions configured
                 */
                SocketOptions &setBufferSizeInBytes(int bufferSize);

            private:
                // socket options

                bool tcpNoDelay;

                bool keepAlive;

                bool reuseAddress;

                int lingerSeconds;

                int bufferSize;

            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_SOCKETOPTIONS_H_ */
