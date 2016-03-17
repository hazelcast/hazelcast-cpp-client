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
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IOException.h"

//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, size_t bufferSize)
                    : IOHandler(connection, oListener), ready(false), informSelector(true), lastMessage(NULL) {
            }


            WriteHandler::~WriteHandler() {
                // no need to delete the messages since they are owned by their associated future objects
            }

            void WriteHandler::run() {
                if (this->connection.live) {
                    informSelector = true;
                    if (ready) {
                        handle();
                    } else {
                        registerHandler();
                    }
                    ready = false;
                }
            }

            // TODO: Add a fragmentation layer here before putting the message into the write queue
            void WriteHandler::enqueueData(protocol::ClientMessage *message) {
                writeQueue.offer(message);
                if (informSelector.compareAndSet(true, false)) {
                    ioSelector.addTask(this);
                    ioSelector.wakeUp();
                }
            }

            void WriteHandler::handle() {
                if (lastMessage == NULL) {
                    lastMessage = writeQueue.poll();
                    if (lastMessage == NULL) {
                        ready = true;
                        return;
                    }

                    if (NULL != lastMessage) {
                        numBytesWrittenToSocketForMessage = 0;
                        lastMessageFrameLen = lastMessage->getFrameLength();
                    }
                }

                while (NULL != lastMessage) {
                    try {
                        numBytesWrittenToSocketForMessage += lastMessage->writeTo(connection.getSocket(),
                                                               numBytesWrittenToSocketForMessage, lastMessageFrameLen);

                        if (numBytesWrittenToSocketForMessage >= lastMessageFrameLen) {
                            // Not deleting message since its memory management is at the future object
                            lastMessage = writeQueue.poll();
                            if (NULL != lastMessage) {
                                numBytesWrittenToSocketForMessage = 0;
                                lastMessageFrameLen = lastMessage->getFrameLength();
                            }
                        } else {
                            // Message could not be sent completely, just continue with another connection
                            break;
                        }
                    } catch (exception::IOException &e) {
                        handleSocketException(e.what());
                        return;
                    }
                }

                ready = false;
                registerHandler();
            }
        }
    }
}

