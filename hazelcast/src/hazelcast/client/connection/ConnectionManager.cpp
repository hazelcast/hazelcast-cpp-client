//
// Created by sancar koyunlu on 8/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClientContext &clientContext, bool smartRouting)
            :clientContext(clientContext)
            , live(true)
            , callIdGenerator(10)
            , iListenerThread(NULL)
            , oListenerThread(NULL)
            , smartRouting(smartRouting) {


            };

            ConnectionManager::~ConnectionManager() {
                shutdown();
            };


            void ConnectionManager::start() {
                socketInterceptor = clientContext.getClientConfig().getSocketInterceptor();
                iListenerThread.reset(new boost::thread(&IListener::listen, &iListener));
                oListenerThread.reset(new boost::thread(&OListener::listen, &oListener));
            }

            void ConnectionManager::shutdown() {
                live = false;
                iListener.shutdown();
                oListener.shutdown();
                iListenerThread->interrupt();
                iListenerThread->join();
                oListenerThread->interrupt();
                oListenerThread->join();
            }

            connection::Connection *ConnectionManager::ownerConnection(const Address &address) {
                Connection *clientConnection = connectTo(address);
                ownerConnectionAddress = clientConnection->getRemoteEndpoint();
                return clientConnection;
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getRandomConnection(int tryCount) {
                Address address = clientContext.getClientConfig().getLoadBalancer()->next().getAddress();
                return getOrConnect(address, tryCount);
            }

            boost::shared_ptr<connection::Connection> ConnectionManager::getOrConnect(const Address &target, int tryCount) {
                checkLive();
                int count = 0;
                exception::IOException lastError("", "");
                while (count < tryCount) {
                    try {
                        if (!clientContext.getClusterService().isMemberExists(target)) {
                            return getRandomConnection();
                        } else {
                            return getOrConnect(target);
                        }
                    } catch (exception::IOException &e) {
                        lastError = e;
                    }
                    count++;
                }
                throw lastError;
            }


            boost::shared_ptr<Connection> ConnectionManager::getOrConnect(const Address &address) {
                checkLive();
                if (smartRouting)
                    return getOrConnectResolved(address);
                else
                    return getOrConnectResolved(ownerConnectionAddress);
            };


            boost::shared_ptr<Connection> ConnectionManager::getOrConnectResolved(const Address &address) {
                boost::shared_ptr<Connection> conn = connections.get(address);
                if (conn.get() == NULL) {
                    boost::lock_guard<boost::mutex> l(lockMutex);
                    conn = connections.get(address);
                    if (conn.get() == NULL) {
                        boost::shared_ptr<Connection> newConnection(connectTo(address));
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

            void ConnectionManager::authenticate(Connection &connection, bool reAuth, bool firstConnection) {
                protocol::AuthenticationRequest auth(clientContext.getClientConfig().getCredentials());
                auth.setPrincipal(principal.get());
                auth.setReAuth(reAuth);
                auth.setFirstConnection(firstConnection);

                connection.init();
                serialization::SerializationService &serializationService = clientContext.getSerializationService();
                serialization::Data authData = serializationService.toData<protocol::AuthenticationRequest>(&auth);
                connection.writeBlocking(authData);

                serialization::Data result = connection.readBlocking();

                boost::shared_ptr<connection::ClientResponse> clientResponse = serializationService.toObject<connection::ClientResponse>(result);
                if (clientResponse->isException())
                    throw exception::IOException("ConnectionManager::authenticate", clientResponse->getException().what());
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::Data *> const &getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(*(getCollection[0]));
                connection.setRemoteEndpoint(*address);
                (std::cout << " --- authenticated by " << *address << " --- " << std::endl);
                if (firstConnection)
                    this->principal = serializationService.toObject<protocol::Principal>(*(getCollection[1]));
            };


            void ConnectionManager::removeConnection(const Address &address) {
                connections.remove(address);
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
                    boost::shared_ptr<util::CallPromise> promise = (*it)->deRegisterEventHandler(callId);
                    if (promise != NULL) {
                        return;
                    }
                }
            }

            connection::Connection *ConnectionManager::connectTo(const Address &address) {
                connection::Connection *conn = new Connection(address, clientContext, iListener, oListener);
                checkLive();
                conn->connect();
                //MTODO socket options
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                authenticate(*conn, true, true);
                return conn;
            }
        }
    }
}