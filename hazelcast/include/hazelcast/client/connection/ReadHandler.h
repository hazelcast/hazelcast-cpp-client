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
// Created by sancar koyunlu on 24/12/13.
//

#ifndef HAZELCAST_ReadHandler
#define HAZELCAST_ReadHandler

#include<stdint.h>

#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        namespace serialization {
        }
        namespace spi {
            class ClientContext;
        }
        namespace connection {
            class Connection;

            class InSelector;

            class HAZELCAST_API ReadHandler : public IOHandler {
            public:
                ReadHandler(Connection &connection, InSelector &iListener, size_t bufferSize,
                            spi::ClientContext &clientContext);

                ~ReadHandler();

                void handle();

                void run();

                int64_t getLastReadTimeMillis();

            private:
                char *buffer;
                util::ByteBuffer byteBuffer;

                protocol::ClientMessageBuilder builder;
                util::Atomic<int64_t> lastReadTimeMillis;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ReadHandler

