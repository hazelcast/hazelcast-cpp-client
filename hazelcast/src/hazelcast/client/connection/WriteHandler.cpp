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
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, int bufferSize)
            : IOHandler(connection, oListener)
            , buffer(bufferSize)
            , lastData(NULL)
            , ready(false)
            , informSelector(true)
            {

            };

            void WriteHandler::run() {
                informSelector = true;
                if (ready) {
                    handle();
                } else {
                    registerHandler();
                }
                ready = false;
            }

            void WriteHandler::enqueueData(const serialization::pimpl::Data &data) {
                serialization::pimpl::DataAdapter *socketWritable = new serialization::pimpl::DataAdapter(data);
                writeQueue.offer(socketWritable);
                if (informSelector.compareAndSet(true, false)) {
                    ioListener.addTask(this);
                    ioListener.wakeUp();
                }
            }

            void WriteHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastWrite = clock();

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

            };
        }
    }
}
