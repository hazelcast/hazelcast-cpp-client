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
            ConnectionManager::ConnectionManager(spi::ClientContext &clientContext)
            :clientContext(clientContext)
//            , heartBeatChecker(clientConfig.getConnectionTimeout(), serializationService)
            , live(true)
            , callIdGenerator(10) {


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

            Connection *ConnectionManager::ownerConnection(const Address &address) {
                return connectTo(address);
            }

            Connection *ConnectionManager::getOrConnect(const Address &address) {
                util::AtomicPointer<Connection> conn = connections.get(address);
                if (conn.isNull()) {
                    boost::lock_guard<boost::mutex> l(lockMutex);
                    conn = connections.get(address);
                    if (conn.isNull()) {
                        Connection *newConnection = connectTo(address);
                        newConnection->getReadHandler().registerSocket();
                        connections.put(newConnection->getRemoteEndpoint(), newConnection);
                        return newConnection;
                    }
                }
                return conn.get();
            };


            Connection *ConnectionManager::getRandomConnection() {
                checkLive();
//        TODO        Address address = clientConfig.getLoadBalancer()->next().getAddress();
                Address address = clientContext.getClientConfig().getAddresses()[0];
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
                Connection *conn = new Connection(address, clientContext, iListener, oListener);
                checkLive();
                conn->connect();
                //TODO socket options
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                authenticate(*conn, true, true);
                return conn;
            }
        }
    }
}