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
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            OutputSocketStream::OutputSocketStream(Socket& socket)
            : socket(socket) {
            }

            void OutputSocketStream::write(const std::vector<byte>& bytes) {
                socket.send((void *)&(bytes[0]), (int)(sizeof(char) * bytes.size()));
            }

            void OutputSocketStream::writeByte(int i) {
                char x = (char)(0xff & i);
                socket.send((void *)&(x), sizeof(char));
            }

            void OutputSocketStream::writeInt(int v) {
                writeByte((v >> 24));
                writeByte((v >> 16));
                writeByte((v >> 8));
                writeByte(v);
            }

            void OutputSocketStream::writeShort(int v) {
                writeByte((v >> 8));
                writeByte(v);
            }
        }
    }
}
