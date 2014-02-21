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
#include "hazelcast/client/exception/IAuthenticationException.h"
#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(spi::ClientContext &clientContext, bool smartRouting)
            :clientContext(clientContext)
            , live(true)
            , callIdGenerator(10)
            , iListener(*this)
            , oListener(*this)
            , iListenerThread(NULL)
            , oListenerThread(NULL)
            , smartRouting(smartRouting) {


            };

            void ConnectionManager::start() {
                socketInterceptor = clientContext.getClientConfig().getSocketInterceptor();
                iListener.start();
                oListener.start();
                iListenerThread.reset(new boost::thread(&InSelector::listen, &iListener));
                oListenerThread.reset(new boost::thread(&OutSelector::listen, &oListener));
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
            }

            connection::Connection *ConnectionManager::ownerConnection(const Address &address) {
                Connection *clientConnection = connectTo(address, true);
                clientConnection->setAsOwnerConnection(true);
                ownerConnectionAddress = clientConnection->getRemoteEndpoint();
                return clientConnection;
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
                } catch (exception::IOException &ignored) {
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
                std::cerr << "Could not connect to any address after 20 try" << std::endl;
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
                else
                    return getOrConnectResolved(ownerConnectionAddress);
            };


            boost::shared_ptr<Connection> ConnectionManager::getOrConnectResolved(const Address &address) {
                boost::shared_ptr<Connection> conn = connections.get(address);
                if (conn.get() == NULL) {
                    boost::lock_guard<boost::mutex> l(lockMutex);
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
                if (clientResponse->isException()) {
                    serialization::Data const &data = clientResponse->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);
                    throw exception::IAuthenticationException("ConnectionManager::authenticate", ex->what());
                }
                boost::shared_ptr<impl::SerializableCollection> collection = serializationService.toObject<impl::SerializableCollection>(clientResponse->getData());
                std::vector<serialization::Data *> const &getCollection = collection->getCollection();
                boost::shared_ptr<Address> address = serializationService.toObject<Address>(*(getCollection[0]));
                connection.setRemoteEndpoint(*address);
                std::stringstream message;
                (message << " --- authenticated by " << address->getHost() << ":" << address->getPort() << " --- " << std::endl);
                util::ILogger::info("ConnectionManager::authenticate", message.str());
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

            connection::Connection *ConnectionManager::connectTo(const Address &address, bool reAuth) {
                std::auto_ptr<connection::Connection> conn(new Connection(address, clientContext, iListener, oListener));
                checkLive();
                conn->connect();
                if (socketInterceptor.get() != NULL) {
                    socketInterceptor.get()->onConnect(conn->getSocket());
                }
                authenticate(*conn, reAuth, reAuth);
                return conn.release();
            }
        }
    }
}