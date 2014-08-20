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
            ConnectionManager::ConnectionManager(spi::ClientContext &clientContext, bool smartRouting)
            :clientContext(clientContext)
            , iListener(*this)
            , oListener(*this)
            , iListenerThread(NULL)
            , oListenerThread(NULL)
            , live(true)
            , callIdGenerator(10)
            , smartRouting(smartRouting)
            , ownerConnectionFuture(clientContext){
                const byte protocol_bytes[6] = {'C', 'B', '1', 'C', 'P', 'P'};
                PROTOCOL.insert(PROTOCOL.begin(), protocol_bytes, protocol_bytes + 6);
            };

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

            void ConnectionManager::shutdown() {
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

            boost::shared_ptr<connection::Connection> ConnectionManager::getOrConnect(const Address &target, int tryCount) {
                checkLive();
                try {
                    if (clientContext.getClusterService().isMemberExists(target)) {
                        boost::shared_ptr<Connection> connection = getOrConnect(target);
                        return connection;
                    }
                } catch (exception::IOException &) {
                }

                int count = 0;
                exception::IOException lastError("", "");
                while (count < tryCount) {
                    try {
                        return getRandomConnection();
                    } catch (exception::IOException &e) {
                        lastError = e;
                    }
                    count++;
                }
                throw lastError;
            }

            boost::shared_ptr<Connection> ConnectionManager::getConnectionIfAvailable(const Address &address) {
                if (!live)
                    return boost::shared_ptr<Connection>();
                return connections.get(address);
            }

            boost::shared_ptr<Connection> ConnectionManager::getOrConnect(const Address &address) {
                checkLive();
                if (smartRouting)
                    return getOrConnectResolved(address);
                else {
                    boost::shared_ptr<Connection> ownerConnPtr = ownerConnectionFuture.getOrWaitForCreation();
                    return getOrConnectResolved(ownerConnPtr->getRemoteEndpoint());
                }
            };


            boost::shared_ptr<Connection> ConnectionManager::getOrConnectResolved(const Address &address) {
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

            void ConnectionManager::authenticate(Connection& connection, bool ownerConnection) {
                protocol::AuthenticationRequest auth(clientContext.getClientConfig().getCredentials());
                auth.setPrincipal(principal.get());
                auth.setFirstConnection(ownerConnection);

                connection.init(PROTOCOL);
                serialization::pimpl::SerializationService &serializationService = clientContext.getSerializationService();
                serialization::pimpl::Data authData = serializationService.toData<protocol::AuthenticationRequest>(&auth);
                connection.writeBlocking(authData);

                serialization::pimpl::Data result = connection.readBlocking();

                boost::shared_ptr<connection::ClientResponse> clientResponse = serializationService.toObject<connection::ClientResponse>(result);
                if (clientResponse->isException()) {
                    serialization::pimpl::Data const &data = clientResponse->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);
                    throw exception::IException("ConnectionManager::authenticate", ex->what());
                }
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::pimpl::Data *> const &getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(*(getCollection[0]));
                connection.setRemoteEndpoint(*address);
                std::stringstream message;
                (message << "client authenticated by " << address->getHost() << ":" << address->getPort());
                util::ILogger::getLogger().info(message.str());
                if (ownerConnection)
                    this->principal = serializationService.toObject<protocol::Principal>(*(getCollection[1]));
            };


            void ConnectionManager::onConnectionClose(const Address& address) {
                connections.remove(address);
                ownerConnectionFuture.closeIfAddressMatches(address);
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

            connection::Connection *ConnectionManager::connectTo(const Address& address, bool ownerConnection) {
                std::auto_ptr<connection::Connection> conn(new Connection(address, clientContext, iListener, oListener));

                checkLive();
                conn->connect();
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                authenticate(*conn, ownerConnection);
                return conn.release();
            }
        }
    }
}
