/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include <winsock2.h>
#endif

#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/impl/BuildInfo.h"

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
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                                   InSelector &iListener, OutSelector &oListener,
                                   internal::socket::SocketFactory &socketFactory)
                    : closedTimeMillis(0), lastHeartbeatRequestedMillis(0), lastHeartbeatReceivedMillis(0),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      readHandler(*this, iListener, 16 << 10, clientContext),
                      writeHandler(*this, oListener, 16 << 10),
                      heartBeating(true), connectionId(connectionId), pendingPacketCount(0),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION) {
                socket = socketFactory.create(address);
            }

            Connection::~Connection() {
            }

            void Connection::incrementPendingPacketCount() {
                ++pendingPacketCount;
            }

            void Connection::decrementPendingPacketCount() {
                --pendingPacketCount;
            }

            int32_t Connection::getPendingPacketCount() {
                return pendingPacketCount;
            }

            void Connection::connect(int timeoutInMillis) {
                if (!isAlive()) {
                    std::ostringstream out;
                    out << "Connection " << (*this) << " is already closed!";
                    throw exception::IOException("Connection::connect", out.str());
                }

                int error = socket->connect(timeoutInMillis);
                if (error) {
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200);
                    throw exception::IOException("Connection::connect", errorMsg);
                }

                // TODO: make this send all guarantee
                socket->send("CB2", 3, MSG_WAITALL);
            }

            void Connection::close(const char *reason) {
                close(reason, boost::shared_ptr<exception::IException>());
            }

            void Connection::close(const char *reason, const boost::shared_ptr<exception::IException> &cause) {
                if (!closedTimeMillis.compareAndSet(0, util::currentTimeMillis())) {
                    return;
                }

                closeCause = cause;
                if (reason) {
                    closeReason = reason;
                }

                logClose();

                writeHandler.deRegisterSocket();
                readHandler.deRegisterSocket();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    util::ILogger::getLogger().warning() << "Exception while closing connection" << e.getMessage();
                }

                clientContext.getConnectionManager().onClose(*this);
            }

            bool Connection::write(const boost::shared_ptr<protocol::ClientMessage> &message) {
                writeHandler.enqueueData(message);
                return true;
            }

            Socket &Connection::getSocket() {
                return *socket;
            }

            const boost::shared_ptr<Address> &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            }

            void Connection::setRemoteEndpoint(const boost::shared_ptr<Address> &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            }

            ReadHandler &Connection::getReadHandler() {
                return readHandler;
            }

            WriteHandler &Connection::getWriteHandler() {
                return writeHandler;
            }

            bool Connection::isHeartBeating() {
                return isAlive() && heartBeating;
            }

            void Connection::handleClientMessage(const boost::shared_ptr<Connection> &connection,
                                                 std::auto_ptr<protocol::ClientMessage> &message) {
                incrementPendingPacketCount();
                if (message->isFlagSet(protocol::ClientMessage::LISTENER_EVENT_FLAG)) {
                    spi::impl::listener::AbstractClientListenerService &listenerService =
                            (spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService();
                    listenerService.handleClientMessage(boost::shared_ptr<protocol::ClientMessage>(message),
                                                        connection);
                } else {
                    invocationService.handleClientMessage(connection, message);
                }
            }

            int Connection::getConnectionId() const {
                return connectionId;
            }

            bool Connection::isAlive() {
                return closedTimeMillis.get() == 0;
            }

            const std::string &Connection::getCloseReason() const {
                return closeReason;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason.empty()) {
                    message << closeReason;
                } else if (closeCause.get() != NULL) {
                    message << closeCause->getSource() << "[" + closeCause->getMessage() << "]";
                } else {
                    message << "Socket explicitly closed";
                }

                util::ILogger &logger = util::ILogger::getLogger();
                if (clientContext.getLifecycleService().isRunning()) {
                    if (!closeCause.get()) {
                        logger.info() << message.str();
                    } else {
                        logger.warning() << message.str() << *closeCause;
                    }
                } else {
                    if (closeCause.get() == NULL) {
                        logger.finest() << message.str();
                    } else {
                        logger.finest() << message.str() << *closeCause;
                    }
                }
            }

            bool Connection::isAuthenticatedAsOwner() {
                return authenticatedAsOwner;
            }

            void Connection::setIsAuthenticatedAsOwner() {
                authenticatedAsOwner = true;
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connectionId == rhs.connectionId;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connectedServerVersionString;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServerVersionString) {
                Connection::connectedServerVersionString = connectedServerVersionString;
                connectedServerVersion = impl::BuildInfo::calculateVersion(connectedServerVersionString);
            }

            int Connection::getConnectedServerVersion() const {
                return connectedServerVersion;
            }

            std::auto_ptr<Address> Connection::getLocalSocketAddress() const {
                return socket->localSocketAddress();
            }

            int64_t Connection::lastReadTimeMillis() {
                return readHandler.getLastReadTimeMillis();
            }

            void Connection::onHeartbeatFailed() {
                heartBeating = false;
            }

            void Connection::onHeartbeatResumed() {
                heartBeating = true;
            }

            void Connection::onHeartbeatReceived() {
                lastHeartbeatReceivedMillis = util::currentTimeMillis();
            }

            void Connection::onHeartbeatRequested() {
                lastHeartbeatRequestedMillis = util::currentTimeMillis();
            }

            void Connection::innerClose() {
                if (!socket.get()) {
                    return;;
                }

                socket->close();
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                Connection &conn = const_cast<Connection &>(connection);
                int64_t lastRead = conn.lastReadTimeMillis();
                int64_t closedTime = conn.closedTimeMillis;
                int64_t lastHeartBeatRequestedTime = conn.lastHeartbeatRequestedMillis;
                int64_t lastHeartBeatReceivedTime = conn.lastHeartbeatReceivedMillis;
                os << "ClientConnection{"
                   << "alive=" << conn.isAlive()
                   << ", connectionId=" << connection.getConnectionId()
                   << ", remoteEndpoint=";
                if (connection.getRemoteEndpoint().get()) {
                    os << *connection.getRemoteEndpoint();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::timeToStringFriendly(lastRead)
                   << ", closedTime=" << util::StringUtil::timeToStringFriendly(closedTime)
                   << ", lastHeartbeatRequested=" << util::StringUtil::timeToStringFriendly(lastHeartBeatRequestedTime)
                   << ", lastHeartbeatReceived=" << util::StringUtil::timeToStringFriendly(lastHeartBeatReceivedTime)
                   << ", isHeartbeating=" << conn.isHeartBeating()
                   << ", connected server version=" << conn.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

