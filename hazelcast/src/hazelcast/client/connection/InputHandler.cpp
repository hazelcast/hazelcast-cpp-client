//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/InputHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/IListener.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include "hazelcast/client/exception/IOException.h"
//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            InputHandler::InputHandler(Connection &connection, IListener &iListener, spi::ClusterService &clusterService, int bufferSize)
            : IOHandler(connection, iListener)
            , clusterService(clusterService)
            , buffer(bufferSize)
            , lastData(NULL) {
                iListener.addTask(this);
            };


            void InputHandler::run() {
                registerHandler();
            }

            void InputHandler::handle() {
                if (!connection.live) {
                    return;
                }
                connection.lastRead = clock();
                try {
                    buffer.readFrom(connection.getSocket());
                } catch (exception::IOException &e) {
                    handleSocketException(e.what());
                    return;
                }

                if (buffer.position() == 0)
                    return;
                buffer.flip();

                while (buffer.hasRemaining()) {
                    if (lastData == NULL) {
                        lastData = new serialization::DataAdapter();
                    }
                    bool complete = lastData->readFrom(buffer);
                    if (complete) {
                        clusterService.handlePacket(connection, lastData->getData());
                        delete lastData;
                        lastData = NULL;
                    } else {
                        break;
                    }
                }

                if (buffer.hasRemaining()) {
                    buffer.compact();
                } else {
                    buffer.clear();
                }
            };
        }
    }
}
