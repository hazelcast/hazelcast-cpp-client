//
// Created by sancar koyunlu on 8/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/impl/ServerException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClientContext& clientContext, bool smartRouting)
            : clientContext(clientContext)
            , inSelector(*this)
            , outSelector(*this)
            , inSelectorThread(NULL)
            , outSelectorThread(NULL)
            , live(true)
            , heartBeater(clientContext)
            , heartBeatThread(NULL)
            , smartRouting(smartRouting)
            , ownerConnectionFuture(clientContext) {
                const byte protocol_bytes[6] = {'C', 'B', '1', 'C', 'P', 'P'};
                PROTOCOL.insert(PROTOCOL.begin(), protocol_bytes, protocol_bytes + 6);
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
                    heartBeatThread->interrupt();
                    heartBeatThread->join();
                    heartBeatThread.reset();
                }
                inSelector.shutdown();
                outSelector.shutdown();
                if (inSelectorThread.get() != NULL) {
                    inSelectorThread->interrupt();
                    inSelectorThread->join();
                    inSelectorThread.reset();
                }
                if (outSelectorThread.get() != NULL) {
                    outSelectorThread->interrupt();
                    outSelectorThread->join();
                    outSelectorThread.reset();
                }
                connections.clear();
            }

            void ConnectionManager::onCloseOwnerConnection() {
                //mark the owner connection as closed so that operations requiring owner connection can be waited.
                ownerConnectionFuture.markAsClosed();
            }

            boost::shared_ptr<Connection> ConnectionManager::createOwnerConnection(const Address& address) {
                return ownerConnectionFuture.createNew(address);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getRandomConnection(int tryCount) {
                Address address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                return getOrConnect(address, tryCount);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getOrConnect(const Address& target, int tryCount) {
                checkLive();
                try {
                    if (clientContext.getClusterService().isMemberExists(target)) {
                        boost::shared_ptr<Connection> connection = getOrConnect(target);
                        return connection;
                    }
                } catch (exception::IOException&) {
                }

                int count = 0;
                exception::IOException lastError("", "");
                while (count < tryCount) {
                    try {
                        return getRandomConnection();
                    } catch (exception::IOException& e) {
                        lastError = e;
                    }
                    count++;
                }
                throw lastError;
            }

            boost::shared_ptr<Connection> ConnectionManager::getConnectionIfAvailable(const Address& address) {
                if (!live)
                    return boost::shared_ptr<Connection>();
                return connections.get(address);
            }

            boost::shared_ptr<Connection> ConnectionManager::getOrConnect(const Address& address) {
                checkLive();
                if (smartRouting) {
                    return getOrConnectResolved(address);
                } else {
                    boost::shared_ptr<Connection> ownerConnPtr = ownerConnectionFuture.getOrWaitForCreation();
                    return getOrConnectResolved(ownerConnPtr->getRemoteEndpoint());
                }
            }


            boost::shared_ptr<Connection> ConnectionManager::getOrConnectResolved(const Address& address) {
                boost::shared_ptr<Connection> conn = connections.get(address);
                if (conn.get() == NULL) {
                    util::LockGuard l(lockMutex);
                    conn = connections.get(address);
                    if (conn.get() == NULL) {
                        boost::shared_ptr<Connection> newConnection(connectTo(address, false));
                        newConnection->getReadHandler().registerSocket();
                        connections.put(newConnection->getRemoteEndpoint(), newConnection);
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

            void ConnectionManager::authenticate(Connection& connection) {
                bool ownerConnection = connection.isOwnerConnection();
                protocol::AuthenticationRequest auth(clientContext.getClientConfig().getCredentials());
                auth.setPrincipal(principal.get());
                auth.setFirstConnection(ownerConnection);

                connection.init(PROTOCOL);
                boost::shared_ptr<ClientResponse> clientResponse = connection.sendAndReceive(auth);
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();

                if (clientResponse->isException()) {
                    serialization::pimpl::Data const& data = clientResponse->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);
                    throw exception::IException("ConnectionManager::authenticate", ex->what());
                }
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::pimpl::Data> const& getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(getCollection[0]);
                connection.setRemoteEndpoint(*address);
                std::stringstream message;
                (message << "client authenticated by " << address->getHost() << ":" << address->getPort());
                util::ILogger::getLogger().info(message.str());
                if (ownerConnection) {
                    this->principal = serializationService.toObject<protocol::Principal>(getCollection[1]);
                }
            }


            void ConnectionManager::onConnectionClose(const Address& address) {
                connections.remove(address);
                ownerConnectionFuture.closeIfAddressMatches(address);
            }

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::InstanceNotActiveException("client");
                }
            }

            connection::Connection *ConnectionManager::connectTo(const Address& address, bool ownerConnection) {
                std::auto_ptr<connection::Connection> conn(new Connection(address, clientContext, inSelector, outSelector, ownerConnection));

                checkLive();
                conn->connect(clientContext.getClientConfig().getConnectionTimeout());
                if (socketInterceptor != NULL) {
                    socketInterceptor->onConnect(conn->getSocket());
                }
                authenticate(*conn);
                return conn.release();
            }


            std::vector<boost::shared_ptr<Connection> > ConnectionManager::getConnections() {
                return connections.values();
            }

            void ConnectionManager::onDetectingUnresponsiveConnection(Connection& connection) {
                if (smartRouting) {
                    //closing the owner connection if unresponsive so that it can be switched to a healthy one.
                    ownerConnectionFuture.closeIfAddressMatches(connection.getRemoteEndpoint());
                    // we do not close connection itself since we will continue to send heartbeat ping to this connection.
                    // IOUtil.closeResource(connection);
                    return;
                }

                //close both owner and operation connection
                ownerConnectionFuture.close();
                util::IOUtil::closeResource(&connection);
            }

            void ConnectionManager::removeEndpoint(const Address& address) {
                boost::shared_ptr<Connection> connection = getConnectionIfAvailable(address);
                if (connection.get() != NULL) {
                    connection->close();
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

