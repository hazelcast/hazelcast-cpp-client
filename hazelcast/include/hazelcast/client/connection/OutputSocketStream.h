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
//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_OUTPUT_SOCKET_STREAM
#define HAZELCAST_OUTPUT_SOCKET_STREAM

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <vector>

namespace hazelcast {
    namespace client {

        class Socket;

        namespace connection {

            class HAZELCAST_API OutputSocketStream {
            public:
                OutputSocketStream(Socket& socket);

                void write(std::vector<byte> bytes, int64_t timeoutInMillis);

                void writeInt(int i);
            private:
                Socket& socket;

                void writeShort(int v);

                void writeByte(int i);
            };

        }
    }
}
#endif /* HAZELCAST_OUTPUT_SOCKET_STREAM */


