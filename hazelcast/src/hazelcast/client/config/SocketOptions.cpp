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
#include "hazelcast/client/config/SocketOptions.h"

namespace hazelcast {
    namespace client {
        namespace config {
            SocketOptions::SocketOptions() : tcpNoDelay(true), keepAlive(true), reuseAddress(true), lingerSeconds(3),
                                             bufferSize(DEFAULT_BUFFER_SIZE_BYTE) {}

            bool SocketOptions::isTcpNoDelay() const {
                return tcpNoDelay;
            }

            SocketOptions &SocketOptions::setTcpNoDelay(bool tcpNoDelay) {
                SocketOptions::tcpNoDelay = tcpNoDelay;
                return *this;
            }

            bool SocketOptions::isKeepAlive() const {
                return keepAlive;
            }

            SocketOptions &SocketOptions::setKeepAlive(bool keepAlive) {
                SocketOptions::keepAlive = keepAlive;
                return *this;
            }

            bool SocketOptions::isReuseAddress() const {
                return reuseAddress;
            }

            SocketOptions &SocketOptions::setReuseAddress(bool reuseAddress) {
                SocketOptions::reuseAddress = reuseAddress;
                return *this;
            }

            int SocketOptions::getLingerSeconds() const {
                return lingerSeconds;
            }

            SocketOptions &SocketOptions::setLingerSeconds(int lingerSeconds) {
                SocketOptions::lingerSeconds = lingerSeconds;
                return *this;
            }

            int SocketOptions::getBufferSizeInBytes() const {
                return bufferSize;
            }

            SocketOptions &SocketOptions::setBufferSizeInBytes(int bufferSize) {
                SocketOptions::bufferSize = bufferSize;
                return *this;
            }

        }
    }
}
