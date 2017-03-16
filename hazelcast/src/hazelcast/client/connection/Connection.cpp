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
// Created by sancar koyunlu on 5/21/13.

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <WinSock2.h>
#endif

#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/util/Util.h"

#ifdef HZ_BUILD_WITH_SSL
#include "hazelcast/client/internal/socket/SSLSocket.h"
#endif

#include <stdint.h>
#include <string.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 4355) 	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener,
                                   OutSelector& oListener, bool isOwner
                    #ifdef HZ_BUILD_WITH_SSL
                    , asio::io_service *ioService, asio::ssl::context *sslContext
                    #endif
                    )
            : live(true)
            , clientContext(clientContext)
            , invocationService(clientContext.getInvocationService())
            , socket(new internal::socket::TcpSocket(address))
            , readHandler(*this, iListener, 16 << 10, clientContext)
            , writeHandler(*this, oListener, 16 << 10)
            , _isOwnerConnection(isOwner)
            , heartBeating(true)
            , receiveBuffer(new byte[16 << 10])
            , receiveByteBuffer((char *)receiveBuffer, 16 << 10)
            , messageBuilder(*this, *this)
            , connectionId(-1) {
                #ifdef HZ_BUILD_WITH_SSL
                if (sslContext) {
                    socket = std::auto_ptr<Socket>(new internal::socket::SSLSocket(address, *ioService, *sslContext));
                } else {
                #endif
                    socket = std::auto_ptr<Socket>(new internal::socket::TcpSocket(address));
                #ifdef HZ_BUILD_WITH_SSL
                }
                #endif
                wrapperMessage.wrapForDecode(receiveBuffer, (int32_t)16 << 10, false);
                assert(receiveByteBuffer.remaining() >= protocol::ClientMessage::HEADER_SIZE); // Note: Always make sure that the size >= ClientMessage header size.
            }

            Connection::~Connection() {
                live = false;
                delete[] receiveBuffer;
            }

            void Connection::connect(int timeoutInMillis) {
                if (!live) {
                    std::ostringstream out;
                    out << "Connection " << connectionId << " is already closed!";
                    throw exception::IOException("Connection::connect", out.str());
                }

                int error = socket->connect(timeoutInMillis);
                if (error) {
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200);
                    throw exception::IOException("Connection::connect", errorMsg);
                }
            }

            void Connection::init(const std::vector<byte>& PROTOCOL) {
                connection::OutputSocketStream outputSocketStream(*socket);
                outputSocketStream.write(PROTOCOL);
            }

            void Connection::close() {
                if (!live.compareAndSet(true, false)) {
                    return;
                }

                const Address &serverAddr = getRemoteEndpoint();
                int socketId = socket->getSocketId();
                
                std::stringstream message;
                message << "Closing connection (id:" << connectionId << ") to " << serverAddr << " with socket id " << socketId <<
                        (_isOwnerConnection ? " as the owner connection." : ".");
                util::ILogger::getLogger().warning(message.str());
                if (!_isOwnerConnection) {
                    readHandler.deRegisterSocket();
                }
                socket->close();
                if (_isOwnerConnection) {
                    return;
                }

                clientContext.getConnectionManager().onConnectionClose(serverAddr, socketId);

                clientContext.getInvocationService().cleanResources(*this);
            }


            void Connection::write(protocol::ClientMessage *message) {
                writeHandler.enqueueData(message);
            }

            Socket& Connection::getSocket() {
                return *socket;
            }

            const Address& Connection::getRemoteEndpoint() const {
                return socket->getRemoteEndpoint();
            }

            void Connection::setRemoteEndpoint(const Address& remoteEndpoint) {
                socket->setRemoteEndpoint(remoteEndpoint);
            }

            std::auto_ptr<protocol::ClientMessage> Connection::sendAndReceive(protocol::ClientMessage &clientMessage) {
                writeBlocking(clientMessage);
                return readBlocking();
            }

            void Connection::writeBlocking(protocol::ClientMessage &message) {
                message.setFlags(protocol::ClientMessage::BEGIN_AND_END_FLAGS);
                int32_t numWritten = 0;
                int32_t frameLen = message.getFrameLength();
                while (numWritten < frameLen) {
                    numWritten += message.writeTo(*socket, numWritten, frameLen);
                }
            }

            std::auto_ptr<protocol::ClientMessage> Connection::readBlocking() {
                responseMessage.reset();
                receiveByteBuffer.clear();
                messageBuilder.reset();

                do {
                    int32_t numRead = 0;
                    do {
                        numRead += receiveByteBuffer.readFrom(*socket,
                                protocol::ClientMessage::VERSION_FIELD_OFFSET - numRead, MSG_WAITALL);
                    } while (numRead < protocol::ClientMessage::VERSION_FIELD_OFFSET); // make sure that we can read the length

                    wrapperMessage.wrapForDecode(receiveBuffer, (int32_t)16 << 10, false);
                    int32_t size = wrapperMessage.getFrameLength();

                    receiveByteBuffer.readFrom(*socket, size - numRead, MSG_WAITALL);

                    receiveByteBuffer.flip();

                    messageBuilder.onData(receiveByteBuffer);

                    receiveByteBuffer.compact();
                } while (NULL == responseMessage.get());

                return responseMessage;
            }

            ReadHandler& Connection::getReadHandler() {
                return readHandler;
            }

            WriteHandler& Connection::getWriteHandler() {
                return writeHandler;
            }

            void Connection::setAsOwnerConnection(bool isOwnerConnection) {
                _isOwnerConnection = isOwnerConnection;
            }

            void Connection::heartBeatingFailed() {
                if (!heartBeating) {
                    return;
                }
                // set the flag first to avoid the usage of this connection
                heartBeating = false;

                std::stringstream errorMessage;
                errorMessage << "Heartbeat to connection  " << getRemoteEndpoint() << " is failed. ";
                util::ILogger::getLogger().warning(errorMessage.str());

                clientContext.getConnectionManager().onDetectingUnresponsiveConnection(*this);
            }

            void Connection::heartBeatingSucceed() {
                heartBeating = true;
            }

            bool Connection::isHeartBeating() {
                return heartBeating;
            }

            void Connection::handleMessage(connection::Connection &connection, std::auto_ptr<protocol::ClientMessage> message) {
                responseMessage = message;
            }

            int  Connection::getConnectionId() const {
                return connectionId;
            }

            void  Connection::setConnectionId(int connectionId) {
                Connection::connectionId = connectionId;
            }

            bool Connection::isOwnerConnection() const {
                return _isOwnerConnection;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

