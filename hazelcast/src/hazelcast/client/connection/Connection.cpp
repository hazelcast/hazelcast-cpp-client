//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/RemoveAllListeners.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/CallFuture.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 4355) 	

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener, OutSelector& oListener)
            : live(true)
            , clientContext(clientContext)
            , invocationService(clientContext.getInvocationService())
            , socket(address)
            , readHandler(*this, iListener, 16 << 10, clientContext)
            , writeHandler(*this, oListener, 16 << 10)
            , _isOwnerConnection(false) {

            }

            Connection::~Connection() {
                live = false;
            }

            void Connection::connect(int timeoutInMillis) {
                int error = socket.connect(timeoutInMillis);
                if (error) {
                    throw exception::IOException("Socket::connect", strerror(error));
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
                message << "Closing connection to " << getRemoteEndpoint();
                util::ILogger::getLogger().finest(message.str());
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
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.writePacket(packet);
            }

            serialization::pimpl::Packet Connection::readBlocking() {
                connection::InputSocketStream inputSocketStream(socket);
                serialization::pimpl::PortableContext& portableContext = clientContext.getSerializationService().getPortableContext();
                inputSocketStream.setPortableContext(&portableContext);
                serialization::pimpl::Packet packet(portableContext);
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
        }
    }
}

#pragma warning(pop)

