//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include <ctime>
//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, int bufferSize)
            : IOHandler(connection, oListener)
            , buffer(bufferSize)
            , lastData(NULL)
            , ready(false)
            , informSelector(true)
            {

            }


            WriteHandler::~WriteHandler() {
                serialization::pimpl::Packet *packet;
                while ((packet = writeQueue.poll()) != NULL) {
                    delete packet;
                }
            }

            void WriteHandler::run() {
                informSelector = true;
                if (ready) {
                    handle();
                } else {
                    registerHandler();
                }
                ready = false;
            }

            void WriteHandler::enqueueData(serialization::pimpl::Packet *packet) {
                writeQueue.offer(packet);
                if (informSelector.compareAndSet(true, false)) {
                    ioSelector.addTask(this);
                    ioSelector.wakeUp();
                }
            }

            void WriteHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastWrite = (int)time(NULL);

                if (lastData == NULL) {
                    lastData = writeQueue.poll();
                    if (lastData == NULL && buffer.position() == 0) {
                        ready = true;
                        return;
                    }
                }
                while (buffer.hasRemaining() && lastData != NULL) {
                    bool complete = lastData->writeTo(buffer);
                    if (complete) {
                        delete lastData;
                        lastData = NULL;
                        lastData = writeQueue.poll();
                    } else {
                        break;
                    }
                }

                if (buffer.position() > 0) {
                    buffer.flip();
                    try {
                        buffer.writeTo(connection.getSocket());
                    } catch (exception::IOException &e) {
                        delete lastData;
                        lastData = NULL;
                        handleSocketException(e.what());
                        return;
                    }
                    if (buffer.hasRemaining()) {
                        buffer.compact();
                    } else {
                        buffer.clear();
                    }
                }
                ready = false;
                registerHandler();

            }
        }
    }
}

