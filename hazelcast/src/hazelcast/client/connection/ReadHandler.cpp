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
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/exception/IOException.h"

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include <ctime>

//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            ReadHandler::ReadHandler(Connection &connection, InSelector &iListener, size_t bufferSize, spi::ClientContext& clientContext)
            : IOHandler(connection, iListener)
            , buffer(new char[bufferSize])
            , byteBuffer(buffer, bufferSize)
            , builder(clientContext.getInvocationService(), connection) {
		        connection.lastRead = (int)time(NULL);
            }

            ReadHandler::~ReadHandler() {
                delete [] buffer;
            }

            void ReadHandler::run() {
                registerHandler();
            }

            void ReadHandler::handle() {
                connection.lastRead = (int)time(NULL);
                try {
                    byteBuffer.readFrom(connection.getSocket());
                } catch (exception::IOException &e) {
                    handleSocketException(e.what());
                    return;
                }

                if (byteBuffer.position() == 0)
                    return;
                byteBuffer.flip();

                while (byteBuffer.hasRemaining()) {
                    builder.onData(byteBuffer);
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }
        }
    }
}

