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
// Created by sancar koyunlu on 8/21/13.

#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/protocol/codec/ClientAuthenticationCodec.h"
#include "hazelcast/client/protocol/codec/ClientAuthenticationCustomCodec.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/client/SocketInterceptor.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClientContext &clientContext, bool smartRouting)
                    : clientContext(clientContext), inSelector(*this), outSelector(*this), inSelectorThread(NULL),
                      outSelectorThread(NULL), live(true), heartBeater(clientContext),
                      heartBeatThread(NULL), smartRouting(smartRouting), ownerConnectionFuture(clientContext),
                      callIdGenerator(0), connectionIdCounter(0) {
                const byte protocol_bytes[3] = {'C', 'B', '2'};
                PROTOCOL.insert(PROTOCOL.begin(), &protocol_bytes[0], &protocol_bytes[3]);
            }

            bool ConnectionManager::start() {
                socketInterceptor = clientContext.getClientConfig().getSocketInterceptor();
                if (!inSelector.start()) {
                    return false;
                }
                if (!outSelector.start()) {
                    return false;
                }
                inSelectorThread.reset(new util::Thread("hz.inListener", InSelector::staticListen, &inSelector));
                outSelectorThread.reset(new util::Thread("hz.outListener", OutSelector::staticListen, &outSelector));
                heartBeatThread.reset(new util::Thread("hz.heartbeater", HeartBeater::staticStart, &heartBeater));
                return true;
            }

            void ConnectionManager::shutdown() {
                live = false;
                heartBeater.shutdown();
                if (heartBeatThread.get() != NULL) {
                    heartBeatThread->cancel();
                    heartBeatThread->join();
                    heartBeatThread.reset();
                }
                inSelector.shutdown();
                outSelector.shutdown();
                if (inSelectorThread.get() != NULL) {
                    inSelectorThread->cancel();
                    inSelectorThread->join();
                    inSelectorThread.reset();
                }
                if (outSelectorThread.get() != NULL) {
                    outSelectorThread->cancel();
                    outSelectorThread->join();
                    outSelectorThread.reset();
                }
                connections.clear();
                socketConnections.clear();
            }

            void ConnectionManager::onCloseOwnerConnection() {
                //mark the owner connection as closed so that operations requiring owner connection can be waited.
                ownerConnectionFuture.markAsClosed();
            }

            boost::shared_ptr<Connection> ConnectionManager::createOwnerConnection(const Address &address) {
                return ownerConnectionFuture.createNew(address);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getRandomConnection(int tryCount) {
                Address address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                return getOrConnect(address, tryCount);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getRandomConnection(int tryCount,
                                                                                             const std::string &lastTriedAddress,
                                                                                             int retryWaitTime) {
                if (!smartRouting) {
                    boost::shared_ptr<Connection> conn = getOwnerConnection();
                    // Check if the retrieved connection is the same as the last one, if so we need to close it so that
                    // a connection to a new member is established.
                    if ((Connection *)NULL != conn.get() &&
                            lastTriedAddress == util::IOUtil::to_string(conn->getRemoteEndpoint())) {
                        // close the connection
                        conn->close();

                        // get the connection again
                        conn = getOwnerConnection();
                    }
                    return conn;
                }

                Address address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                std::string newAddr = util::IOUtil::to_string(address);
                if (newAddr == lastTriedAddress) {
                    address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                }
                newAddr = util::IOUtil::to_string(address);
                if (newAddr == lastTriedAddress) {
                    util::sleep(retryWaitTime);
                }
                return getOrConnect(address, tryCount);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getOrConnect(const Address &target,
                                                                                      int tryCount) {
                checkLive();

                try {
                    if (clientContext.getClusterService().isMemberExists(target)) {
                        boost::shared_ptr<Connection> connection = getOrConnect(target);
                        // Only return the live connections
                        if (connection->live) {
                            return connection;
                        }
                    }
                } catch (exception::IException &e) {
                    if (tryCount <= 0) {
                        throw e;
                    }
                }

                int count = 0;
                while (true) {
                    try {
                        boost::shared_ptr<Connection> conn = getRandomConnection();
                        if (conn.get() != (Connection *) NULL && conn->live) {
                            return conn;
                        }
                    } catch (exception::IException &e) {
                        ++count;
                        if (count >= tryCount) {
                            throw e;
                        }
                    }
                }
            }

            boost::shared_ptr<Connection> ConnectionManager::getConnectionIfAvailable(const Address &address) {
                if (!live)
                    return boost::shared_ptr<Connection>();
                return connections.get(address);
            }

            boost::shared_ptr<Connection> ConnectionManager::getConnectionIfAvailable(int socketDescriptor) {
                if (!live)
                    return boost::shared_ptr<Connection>();
                return socketConnections.get(socketDescriptor);
            }

            boost::shared_ptr<Connection> ConnectionManager::getOrConnect(const Address &address) {
                checkLive();
                if (smartRouting) {
                    return getOrConnectResolved(address);
                }

                return getOwnerConnection();
            }

            boost::shared_ptr<Connection> ConnectionManager::getOwnerConnection() {
                boost::shared_ptr<Connection> ownerConnPtr = ownerConnectionFuture.getOrWaitForCreation();
                return getOrConnectResolved(ownerConnPtr->getRemoteEndpoint());
            }


            boost::shared_ptr<Connection> ConnectionManager::getOrConnectResolved(const Address &address) {
                boost::shared_ptr<Connection> conn = connections.get(address);
                if (conn.get() == NULL) {
                    util::LockGuard l(lockMutex);
                    conn = connections.get(address);
                    if (conn.get() == NULL) {
                        boost::shared_ptr<Connection> newConnection(connectTo(address, false));
                        newConnection->getReadHandler().registerSocket();
                        connections.put(newConnection->getRemoteEndpoint(), newConnection);
                        socketConnections.put(newConnection->getSocket().getSocketId(), newConnection);
                        return newConnection;
                    }
                }
                return conn;
            }

            boost::shared_ptr<Connection> ConnectionManager::getRandomConnection() {
                checkLive();
                Address address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                return getOrConnect(address);
            }

            void ConnectionManager::authenticate(Connection *connection) {
                const Credentials *credentials = clientContext.getClientConfig().getCredentials();

                std::auto_ptr<protocol::ClientMessage> authenticationMessage;
                byte serializationVersion = clientContext.getSerializationService().getVersion();

                // get principal as a shared_ptr first since it may be changed concurrently
                boost::shared_ptr<protocol::Principal> latestPrincipal = principal;
                if (NULL == credentials) {
                    GroupConfig &groupConfig = clientContext.getClientConfig().getGroupConfig();
                    const protocol::UsernamePasswordCredentials cr(groupConfig.getName(), groupConfig.getPassword());
                    authenticationMessage = protocol::codec::ClientAuthenticationCodec::RequestParameters::encode(
                            cr.getPrincipal(), cr.getPassword(),
                            latestPrincipal.get() ? latestPrincipal->getUuid() : NULL,
                            latestPrincipal.get() ? latestPrincipal->getOwnerUuid() : NULL,
                            connection->isOwnerConnection(), protocol::ClientTypes::CPP, serializationVersion);
                } else {
                    serialization::pimpl::Data data =
                            clientContext.getSerializationService().toData<Credentials>(credentials);

                    authenticationMessage = protocol::codec::ClientAuthenticationCustomCodec::RequestParameters::encode(
                            data, latestPrincipal.get() ? latestPrincipal->getUuid() : NULL,
                            latestPrincipal.get() ? latestPrincipal->getOwnerUuid() : NULL,
                            connection->isOwnerConnection(), protocol::ClientTypes::CPP, serializationVersion);
                }

                connection->init(PROTOCOL);

                authenticationMessage->setCorrelationId(getNextCallId());

                std::auto_ptr<protocol::ClientMessage> clientResponse = connection->sendAndReceive(
                        *authenticationMessage);

                if (protocol::EXCEPTION == clientResponse->getMessageType()) {
                    protocol::codec::ErrorCodec errorResponse = protocol::codec::ErrorCodec::decode(*clientResponse);
                    throw exception::IException("ConnectionManager::authenticate", std::string(
                            "Authentication error message from server ! ") + errorResponse.toString());
                }

                if (NULL == credentials) {
                    protocol::codec::ClientAuthenticationCodec::ResponseParameters resultParameters = protocol::codec::ClientAuthenticationCodec::ResponseParameters::decode(
                            *clientResponse);

                    protocol::AuthenticationStatus authenticationStatus = (protocol::AuthenticationStatus) resultParameters.status;
                    switch (authenticationStatus) {
                        case protocol::AUTHENTICATED:
                        {
                            processSuccessfulAuthenticationResult(connection, resultParameters.address,
                                                                  resultParameters.uuid,
                                                                  resultParameters.ownerUuid);
                            break;
                        }
                        case protocol::CREDENTIALS_FAILED:
                        {
                            throw exception::AuthenticationException("ConnectionManager::authenticate",
                                                                               "Invalid credentials!");
                        }
                        case protocol::SERIALIZATION_VERSION_MISMATCH:
                        {
                            //we do not need serialization version here as we already connected to master and agreed on the version
                            char msg[100];
                            util::snprintf(msg, 100, "Serialization version does not match the server side. client serailization version:%d",
                                    serializationVersion);
                            throw exception::AuthenticationException("ConnectionManager::authenticate", msg);
                        }
                        default:
                        {
                            //we do not need serialization version here as we already connected to master and agreed on the version
                            char msg[70];
                            util::snprintf(msg, 70, "Authentication status code not supported. status:%d",
                                    resultParameters.status);
                            throw exception::AuthenticationException("ConnectionManager::authenticate", msg);
                        }
                    }
                } else {
                    protocol::codec::ClientAuthenticationCustomCodec::ResponseParameters resultParameters = protocol::codec::ClientAuthenticationCustomCodec::ResponseParameters::decode(
                            *clientResponse);

                    protocol::AuthenticationStatus authenticationStatus = (protocol::AuthenticationStatus) resultParameters.status;
                    switch (authenticationStatus) {
                        case protocol::AUTHENTICATED:
                        {
                            processSuccessfulAuthenticationResult(connection, resultParameters.address,
                                                                  resultParameters.uuid,
                                                                  resultParameters.ownerUuid);
                            break;
                        }
                        case protocol::CREDENTIALS_FAILED:
                        {
                            throw exception::AuthenticationException("ConnectionManager::authenticate",
                                                                               "Invalid credentials!");
                        }
                        case protocol::SERIALIZATION_VERSION_MISMATCH:
                        {
                            //we do not need serialization version here as we already connected to master and agreed on the version
                            char msg[100];
                            util::snprintf(msg, 100, "Serialization version does not match the server side. client serailization version:%d",
                                    serializationVersion);
                            throw exception::AuthenticationException("ConnectionManager::authenticate", msg);
                        }
                        default:
                        {
                            //we do not need serialization version here as we already connected to master and agreed on the version
                            char msg[70];
                            util::snprintf(msg, 70, "Authentication status code not supported. status:%d",
                                    resultParameters.status);
                            throw exception::AuthenticationException("ConnectionManager::authenticate", msg);
                        }
                    }
                }
            }

            void ConnectionManager::onConnectionClose(const Address &address, int socketId) {
                socketConnections.remove(socketId);
                connections.remove(address);
                ownerConnectionFuture.closeIfAddressMatches(address);
            }

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::HazelcastInstanceNotActiveException("client");
                }
            }

            std::auto_ptr<Connection> ConnectionManager::connectTo(const Address &address, bool ownerConnection) {
                std::auto_ptr<connection::Connection> conn(
                        new Connection(address, clientContext, inSelector, outSelector, ownerConnection));

                checkLive();
                conn->connect(clientContext.getClientConfig().getConnectionTimeout());
                if (socketInterceptor != NULL) {
                    socketInterceptor->onConnect(conn->getSocket());
                }

                authenticate(conn.get());
                return conn;
            }


            std::vector<boost::shared_ptr<Connection> > ConnectionManager::getConnections() {
                return connections.values();
            }

            void ConnectionManager::onDetectingUnresponsiveConnection(Connection &connection) {
                if (smartRouting) {
                    //closing the owner connection if unresponsive so that it can be switched to a healthy one.
                    ownerConnectionFuture.closeIfAddressMatches(connection.getRemoteEndpoint());
                } else {
                    ownerConnectionFuture.close();
                }
                util::IOUtil::closeResource(&connection);
            }

            void ConnectionManager::removeEndpoint(const Address &address) {
                boost::shared_ptr<Connection> connection = getConnectionIfAvailable(address);
                if (connection.get() != NULL) {
                    connection->close();
                }
            }

            int64_t ConnectionManager::getNextCallId() {
                return ++callIdGenerator;
            }

            void ConnectionManager::processSuccessfulAuthenticationResult(Connection *connection,
                                                                          std::auto_ptr<Address> addr,
                                                                          std::auto_ptr<std::string> uuid,
                                                                          std::auto_ptr<std::string> ownerUuid) {
                connection->setRemoteEndpoint(*addr);
                connection->setConnectionId(++connectionIdCounter);

                std::stringstream message;
                message << "Connected and authenticated by " << *addr << ". Connection id:"
                        << connection->getConnectionId() << " , socket id:" << connection->getSocket().getSocketId();

                if (connection->isOwnerConnection()) {
                    principal = std::auto_ptr<protocol::Principal>(new protocol::Principal(uuid, ownerUuid));
                    message << " as owner connection";
                } else {
                    connection->getSocket().setBlocking(false);
                }

                if ((protocol::Principal *) NULL != principal.get()) {
                    const std::string *clientUuid = principal->getUuid();
                    if (NULL != clientUuid) {
                        message << ". Client uuid: " << *clientUuid;
                    } else {
                        message << ". Client uuid is NULL";
                    }
                    const std::string *ownerMemberUuid = principal->getOwnerUuid();
                    if (NULL != ownerMemberUuid) {
                        message << ". Owner member uuid: " << *ownerMemberUuid;
                    } else {
                        message << ". Owner member uuid is NULL";
                    }
                } else {
                    message << ". No principal exist!!!";
                }

                util::ILogger::getLogger().info(message.str());
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

