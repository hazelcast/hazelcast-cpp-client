//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
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
            , lastData(NULL)
            , clientContext(clientContext){

            }

            ReadHandler::~ReadHandler() {
                delete buffer;
            }

            void ReadHandler::run() {
                registerHandler();
            }

            void ReadHandler::handle() {
                if (!connection.live) {
                    return;
                }
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
                    if (lastData == NULL) {
                        lastData = new serialization::pimpl::Packet(getPortableContext());
                    }
                    bool complete = lastData->readFrom(byteBuffer);
                    if (complete) {
                        clientContext.getInvocationService().handlePacket(connection, *lastData);
                        delete lastData;
                        lastData = NULL;
                    } else {
                        break;
                    }
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }


            serialization::pimpl::PortableContext& ReadHandler::getPortableContext() {
                return clientContext.getSerializationService().getPortableContext();
            }
        }
    }
}

