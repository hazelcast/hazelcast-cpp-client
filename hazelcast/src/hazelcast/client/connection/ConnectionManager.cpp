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
#include "CallPromise.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClusterService &clusterService, serialization::SerializationService &serializationService, ClientConfig &clientConfig)
            : clusterService(clusterService)
            , serializationService(serializationService)
            , clientConfig(clientConfig)
//            , heartBeatChecker(clientConfig.getConnectionTimeout(), serializationService)
            , socketInterceptor(this->clientConfig.getSocketInterceptor())
            , iListenerThread(new boost::thread(&IListener::listen, &iListener))
            , oListenerThread(new boost::thread(&OListener::listen, &oListener))
            , live(true)
            , callIdGenerator(0) {


            };

            ConnectionManager::~ConnectionManager() {
                shutdown();
            };

            void ConnectionManager::shutdown() {
                live = false;
                iListener.shutdown();
                oListener.shutdown();
                iListenerThread->join();
                oListenerThread->join();
            }

            Connection *ConnectionManager::ownerConnection(const Address &address) {
                return getOrConnect(address);
            }

            Connection *ConnectionManager::getOrConnect(const Address &address) {
                util::AtomicPointer<Connection> conn = connections.get(address);
                if (conn.isNull()) {
                    boost::lock_guard<boost::mutex> l(lockMutex);
                    conn = connections.get(address);
                    if (conn.isNull()) {
                        Connection *newConnection = connectTo(address);
                        authenticate(*newConnection, true, true);
                        connections.put(newConnection->getRemoteEndpoint(), newConnection);
                        return newConnection;
                    }
                }
                return conn.get();
            };


            Connection *ConnectionManager::getRandomConnection() {
                checkLive();
//        TODO        Address address = clientConfig.getLoadBalancer()->next().getAddress();
                Address address = clientConfig.getAddresses()[0];
                return getOrConnect(address);
            }

            void ConnectionManager::authenticate(Connection &connection, bool reAuth, bool firstConnection) {
                protocol::AuthenticationRequest auth(clientConfig.getCredentials());
                auth.setPrincipal(principal.get());
                auth.setReAuth(reAuth);
                auth.setFirstConnection(firstConnection);

                connection.init();
                serialization::Data authData = serializationService.toData<protocol::AuthenticationRequest>(&auth);
                connection.writeBlocking(authData);

                serialization::Data result = connection.readBlocking();

                boost::shared_ptr<connection::ClientResponse> clientResponse = serializationService.toObject<connection::ClientResponse>(result);
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::Data *> const &getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(*(getCollection[0]));
                connection.setRemoteEndpoint(*address);
                std::cout << " --- authenticated ----- " << std::endl;
                if (firstConnection)
                    this->principal = serializationService.toObject<protocol::Principal>(*(getCollection[1]));
            };

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw exception::InstanceNotActiveException("ConnectionManager::checkLive", "Instance is not active");
                }
            }

            void ConnectionManager::destroyConnection(Connection &connection) {
                Address const &endpoint = connection.getRemoteEndpoint();
//                if (endpoint != null) { TODO
//                    connections.remove(clientConnection.getRemoteEndpoint());
//                }
                connection.removeConnectionCalls();
            }


            int ConnectionManager::getNextCallId() {
                return callIdGenerator++;
            }


            void ConnectionManager::removeEventHandler(int callId) {
                std::vector<util::AtomicPointer<Connection> > v = connections.values();
                std::vector<util::AtomicPointer<Connection> >::iterator it;
                for (it = v.begin(); it != v.end(); ++it) {
                    util::CallPromise *promise = (*it)->deRegisterEventHandler(callId);
                    if (promise != NULL) {
                        //TODO delete promise;
                        return;
                    }
                }

            }

            Connection *ConnectionManager::connectTo(const Address &address) {
                Connection *conn = new Connection(address, *this, serializationService, clusterService, iListener, oListener);
                checkLive();
                conn->connect();
                //TODO socket options
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                return conn;
            }
        }
    }
}