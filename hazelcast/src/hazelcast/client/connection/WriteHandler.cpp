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
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, size_t bufferSize)
            : IOHandler(connection, oListener)
            , buffer(new char[bufferSize])
            , byteBuffer(buffer, bufferSize)
            , lastData(NULL)
            , ready(false)
            , informSelector(true)
            {
		connection.lastWrite = (int)time(NULL);
	    }


            WriteHandler::~WriteHandler() {
                serialization::pimpl::Packet *packet;
                while ((packet = writeQueue.poll()) != NULL) {
                    delete packet;
                }
                delete [] buffer;
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
                    if (lastData == NULL && byteBuffer.position() == 0) {
                        ready = true;
                        return;
                    }
                }
                while (byteBuffer.hasRemaining() && lastData != NULL) {
                    bool complete = lastData->writeTo(byteBuffer);
                    if (complete) {
                        delete lastData;
                        lastData = NULL;
                        lastData = writeQueue.poll();
                    } else {
                        break;
                    }
                }

                if (byteBuffer.position() > 0) {
                    byteBuffer.flip();
                    try {
                        byteBuffer.writeTo(connection.getSocket());
                    } catch (exception::IOException &e) {
                        delete lastData;
                        lastData = NULL;
                        handleSocketException(e.what());
                        return;
                    }
                    if (byteBuffer.hasRemaining()) {
                        byteBuffer.compact();
                    } else {
                        byteBuffer.clear();
                    }
                }
                ready = false;
                registerHandler();

            }
        }
    }
}

