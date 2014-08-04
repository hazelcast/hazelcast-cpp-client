//
// Created by sancar koyunlu on 8/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/ServerException.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClientContext& clientContext, bool smartRouting)
            : clientContext(clientContext)
            , iListener(*this)
            , oListener(*this)
            , iListenerThread(NULL)
            , oListenerThread(NULL)
            , live(true)
            , callIdGenerator(10)
            , smartRouting(smartRouting) {
                const byte protocol_bytes[6] = {'C', 'B', '1', 'C', 'P', 'P'};
                PROTOCOL.insert(PROTOCOL.begin(), protocol_bytes, protocol_bytes + 6);
            }

            bool ConnectionManager::start() {
                socketInterceptor = clientContext.getClientConfig().getSocketInterceptor();
                if (!iListener.start()) {
                    return false;
                }
                if (!oListener.start()) {
                    return false;
                }
                iListenerThread.reset(new util::Thread("hz.inListener", InSelector::staticListen, &iListener));
                oListenerThread.reset(new util::Thread("hz.outListener", OutSelector::staticListen, &oListener));
                return true;
            }

            void ConnectionManager::stop() {
                live = false;
                iListener.shutdown();
                oListener.shutdown();
                if (iListenerThread.get() != NULL) {
                    iListenerThread->interrupt();
                    iListenerThread->join();
                }
                if (oListenerThread.get() != NULL) {
                    oListenerThread->interrupt();
                    oListenerThread->join();
                }
                connections.clear();
            }

            void ConnectionManager::markOwnerAddressAsClosed() {
                ownerConnection.reset();
            }

            Address ConnectionManager::waitForOwnerConnection() {
                if (ownerConnection.get() != NULL) {
                    return ownerConnection->getRemoteEndpoint();
                }
                ClientConfig& config = clientContext.getClientConfig();
                int tryCount = 2 * config.getAttemptPeriod() * config.getConnectionAttemptLimit() / 1000;
                while (ownerConnection.get() == NULL) {
                    util::sleep(1);
                    if (--tryCount == 0) {
                        throw exception::IOException("ConnectionManager", "Wait for owner connection is timed out");
                    }
                }
                return ownerConnection->getRemoteEndpoint();
            }

            boost::shared_ptr<Connection> ConnectionManager::createOwnerConnection(const Address& address) {
                Connection *clientConnection = connectTo(address, true);
                clientConnection->setAsOwnerConnection(true);
                ownerConnection.reset(clientConnection);
                return ownerConnection;
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
                if (smartRouting)
                    return getOrConnectResolved(address);
                else {
                    return getOrConnectResolved(waitForOwnerConnection());
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

            void ConnectionManager::authenticate(Connection& connection, bool reAuth, bool firstConnection) {
                protocol::AuthenticationRequest auth(clientContext.getClientConfig().getCredentials());
                auth.setPrincipal(principal.get());
                auth.setReAuth(reAuth);
                auth.setFirstConnection(firstConnection);

                connection.init(PROTOCOL);
                boost::shared_ptr<ClientResponse> clientResponse = connection.sendAndReceive(auth);
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();

                if (clientResponse->isException()) {
                    serialization::pimpl::Data const& data = clientResponse->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);
                    throw exception::IException("ConnectionManager::authenticate", ex->what());
                }
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::pimpl::Data *> const& getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(*(getCollection[0]));
                connection.setRemoteEndpoint(*address);
                std::stringstream message;
                (message << "client authenticated by " << address->getHost() << ":" << address->getPort());
                util::ILogger::getLogger().info(message.str());
                if (firstConnection)
                    this->principal = serializationService.toObject<protocol::Principal>(*(getCollection[1]));
            }


            void ConnectionManager::destroyConnection(const Address& address) {
                connections.remove(address);
                closeIfOwnerConnection(address);
            }


            void ConnectionManager::closeIfOwnerConnection(const Address& address) {
                boost::shared_ptr<Connection> currentOwnerConnection = ownerConnection;

                if (currentOwnerConnection.get() == NULL || !currentOwnerConnection->live) {
                    return;
                }
                std::stringstream message;
                message << "closing owner connection to " << address;
                util::ILogger::getLogger().finest(message.str());
                if (currentOwnerConnection->getRemoteEndpoint() == address) {
                    currentOwnerConnection->close();
                }
            }

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::InstanceNotActiveException("client");
                }
            }

            int ConnectionManager::getNextCallId() {
                return callIdGenerator++;
            }

            void ConnectionManager::removeEventHandler(int callId) {
                std::vector<boost::shared_ptr<Connection> > v = connections.values();
                std::vector<boost::shared_ptr<Connection> >::iterator it;
                for (it = v.begin(); it != v.end(); ++it) {
                    boost::shared_ptr<CallPromise> promise = (*it)->deRegisterEventHandler(callId);
                    if (promise != NULL) {
                        return;
                    }
                }
            }

            connection::Connection *ConnectionManager::connectTo(const Address& address, bool reAuth) {
                std::auto_ptr<connection::Connection> conn(new Connection(address, clientContext, iListener, oListener));

                checkLive();
                conn->connect(clientContext.getClientConfig().getConnectionTimeout());
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                authenticate(*conn, reAuth, reAuth);
                return conn.release();
            }
        }
    }
}
