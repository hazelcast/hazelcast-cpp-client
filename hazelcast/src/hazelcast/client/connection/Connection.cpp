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
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#include <string.h>
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/impl/RemoveAllListeners.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/CallFuture.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 4355) 	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener, OutSelector& oListener, bool isOwner)
            : live(true)
            , clientContext(clientContext)
            , invocationService(clientContext.getInvocationService())
            , socket(address)
            , readHandler(*this, iListener, 16 << 10, clientContext)
            , writeHandler(*this, oListener, 16 << 10)
            , _isOwnerConnection(isOwner)
            , receiveBuffer(new char[16 << 10])
            , receiveByteBuffer(receiveBuffer, 16 << 10) {

            }

            Connection::~Connection() {
                live = false;
            }

            void Connection::connect(int timeoutInMillis) {
                int error = socket.connect(timeoutInMillis);
                if (error) {
                    throw exception::IOException("Socket::connect", strerror(error));
                } else {
                    std::stringstream message;
                    message << "Connected to " << socket.getAddress() << " with socket id " << socket.getSocketId() <<
                            (_isOwnerConnection ? " as the owner connection." : ".");
                    util::ILogger::getLogger().info(message.str());
                }
            }

            void Connection::init(const std::vector<byte>& PROTOCOL) {
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.write(PROTOCOL);
            }

            void Connection::close() {
                if (!live.compareAndSet(true, false)) {
                    return;
                }

                std::stringstream message;
                message << "Closing connection to " << getRemoteEndpoint() << " with socket id " << socket.getSocketId() <<
                        (_isOwnerConnection ? " as the owner connection." : ".");
                util::ILogger::getLogger().warning(message.str());
                if (!_isOwnerConnection) {
                    readHandler.deRegisterSocket();
                }
                socket.close();
                if (_isOwnerConnection) {
                    return;
                }

                clientContext.getConnectionManager().onConnectionClose(socket.getRemoteEndpoint());
                clientContext.getInvocationService().cleanResources(*this);
            }


            void Connection::write(serialization::pimpl::Packet *packet) {
                writeHandler.enqueueData(packet);
            }

            Socket& Connection::getSocket() {
                return socket;
            }

            const Address& Connection::getRemoteEndpoint() const {
                return socket.getRemoteEndpoint();
            }

            void Connection::setRemoteEndpoint(Address& remoteEndpoint) {
                socket.setRemoteEndpoint(remoteEndpoint);
            }

            boost::shared_ptr<connection::ClientResponse> Connection::sendAndReceive(const impl::ClientRequest& clientRequest) {
                serialization::pimpl::SerializationService& service = clientContext.getSerializationService();
                serialization::pimpl::Data request = service.toData<impl::ClientRequest>(&clientRequest);
                serialization::pimpl::Packet packet(service.getPortableContext(), request);
                writeBlocking(packet);
                serialization::pimpl::Packet responsePacket = readBlocking();
                return service.toObject<ClientResponse>(responsePacket.getData());
            }

            void Connection::writeBlocking(serialization::pimpl::Packet const& packet) {
                OutputSocketStream outputSocketStream(socket);
                outputSocketStream.writePacket(packet);
            }

            serialization::pimpl::Packet Connection::readBlocking() {
                connection::InputSocketStream inputSocketStream(socket);
                serialization::pimpl::PortableContext& portableContext = clientContext.getSerializationService().getPortableContext();
                inputSocketStream.setPortableContext(&portableContext);
                std::auto_ptr<std::vector<byte> > buffer(new std::vector<byte>());
                serialization::pimpl::Data data(buffer);
                serialization::pimpl::Packet packet(portableContext, data);
                inputSocketStream.readPacket(packet);
                return packet;
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
                std::stringstream errorMessage;
                if (!heartBeating) {
                    return;
                }
                errorMessage << "Heartbeat to connection  " << getRemoteEndpoint() << " is failed. ";
                util::ILogger::getLogger().warning(errorMessage.str());

                impl::RemoveAllListeners *request = new impl::RemoveAllListeners();
                spi::InvocationService& invocationService = clientContext.getInvocationService();
                invocationService.invokeOnTarget(request, getRemoteEndpoint());
                heartBeating = false;
                ConnectionManager& connectionManager = clientContext.getConnectionManager();
                connectionManager.onDetectingUnresponsiveConnection(*this);

                //Other resources(request promises) will be handled by CallFuture.get()
                invocationService.cleanEventHandlers(*this);
            }

            void Connection::heartBeatingSucceed() {
                heartBeating = true;
            }

            bool Connection::isHeartBeating() {
                return heartBeating;
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

