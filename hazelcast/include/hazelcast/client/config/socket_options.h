/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/util/export.h"

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
            class HAZELCAST_API socket_options {
            public:
                /**
                 * constant for kilobyte
                 */
                static const int KILO_BYTE = 1024;

                /**
                 * default buffer size of Bytes
                 */
                static const int DEFAULT_BUFFER_SIZE_BYTE = 128 * KILO_BYTE;

                socket_options();

                /**
                 * TCP_NODELAY socket option
                 *
                 * @return true if enabled
                 */
                bool is_tcp_no_delay() const;

                /**
                 * Enable/disable TCP_NODELAY socket option.
                 *
                 * @param tcpNoDelay
                 */
                socket_options &set_tcp_no_delay(bool tcp_no_delay);

                /**
                 * SO_KEEPALIVE socket option
                 *
                 * @return true if enabled
                 */
                bool is_keep_alive() const;

                /**
                 * Enable/disable SO_KEEPALIVE socket option.
                 *
                 * @param keepAlive enabled if true
                 * @return SocketOptions configured
                 */
                socket_options &set_keep_alive(bool keep_alive);

                /**
                 * SO_REUSEADDR socket option.
                 *
                 * @return true if enabled
                 */
                bool is_reuse_address() const;

                /**
                 * Enable/disable the SO_REUSEADDR socket option.
                 *
                 * @param reuseAddress enabled if true
                 * @return SocketOptions configured
                 */
                socket_options &set_reuse_address(bool reuse_address);

                /**
                 * Gets SO_LINGER with the specified linger time in seconds
                 * @return lingerSeconds value in seconds
                 */
                int get_linger_seconds() const;

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
                socket_options &set_linger_seconds(int linger_seconds);

                /**
                 * If set to 0 or less, then it is not set on the socket.
                 *
                 * The default value is DEFAULT_BUFFER_SIZE_BYTE
                 *
                 * Gets the SO_SNDBUF and SO_RCVBUF options value in bytes
                 * @return bufferSize Number of bytes
                 */
                int get_buffer_size_in_bytes() const;

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
                socket_options &set_buffer_size_in_bytes(int buffer_size);

            private:
                // socket options

                bool tcp_no_delay_;

                bool keep_alive_;

                bool reuse_address_;

                int linger_seconds_;

                int buffer_size_;

            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


