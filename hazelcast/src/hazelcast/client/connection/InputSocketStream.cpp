/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 5/10/13.

#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            InputSocketStream::InputSocketStream(Socket& socket)
            : socket(socket) {
            }

            int InputSocketStream::readInt() {
                byte s[4];
                socket.receive(s, sizeof(byte) * 4, MSG_WAITALL);
                return (0xff000000 & (s[0] << 24)) |
                (0x00ff0000 & (s[1] << 16)) |
                (0x0000ff00 & (s[2] << 8)) |
                (0x000000ff & s[3]);
            }


            int InputSocketStream::readShort() {
                byte s[2];
                socket.receive(s, sizeof(byte) * 2, MSG_WAITALL);
                return (0xff00 & (s[0] << 8)) |
                (0x00ff & s[1]);
            }


            int InputSocketStream::readByte() {
                byte s;
                socket.receive(&s, sizeof(byte), MSG_WAITALL);
                return s;
            }

            bool InputSocketStream::readBoolean() {
                return (bool)readByte();
            }
        }
    }
}

