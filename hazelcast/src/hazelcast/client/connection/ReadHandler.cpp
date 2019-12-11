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

#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/util/Util.h"

#include <ctime>

//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, InSelector &iListener, size_t bufferSize, spi::ClientContext& clientContext)
            : IOHandler(connection, iListener)
            , buffer(new char[bufferSize])
            , byteBuffer(buffer, bufferSize)
            , builder(connection) {
		        lastReadTimeMillis = util::currentTimeMillis();
            }

            ReadHandler::~ReadHandler() {
                delete [] buffer;
            }

            void ReadHandler::run() {
                registerHandler();
            }

            void ReadHandler::handle() {
                lastReadTimeMillis = util::currentTimeMillis();
                try {
                    byteBuffer.readFrom(connection.getSocket());
                } catch (exception::IOException &e) {
                    handleSocketException(e.what());
                    return;
                }

                if (byteBuffer.position() == 0)
                    return;
                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            int64_t ReadHandler::getLastReadTimeMillis() {
                return lastReadTimeMillis;
            }
        }
    }
}

