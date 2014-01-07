//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/MemberShipEvent.h"
#include "hazelcast/client/exception/TargetDisconnectedException.h"
#include "hazelcast/client/impl/EventHandlerWrapper.h"
#include "hazelcast/util/CallPromise.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(spi::PartitionService &partitionService, spi::LifecycleService &lifecycleService, connection::ConnectionManager &connectionManager, serialization::SerializationService &serializationService, ClientConfig &clientConfig)
            : partitionService(partitionService)
            , lifecycleService(lifecycleService)
            , connectionManager(connectionManager)
            , serializationService(serializationService)
            , clientConfig(clientConfig)
            , clusterThread(connectionManager, clientConfig, *this, lifecycleService, serializationService)
            , credentials(clientConfig.getCredentials())
            , redoOperation(clientConfig.isRedoOperation())
            , active(false) {

            }

            void ClusterService::start() {
                serialization::ClassDefinitionBuilder cd(-3, 3);
                serialization::ClassDefinition *ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
                serializationService.getSerializationContext().registerClassDefinition(ptr);

                boost::thread *t = new boost::thread(boost::bind(&connection::ClusterListenerThread::run, &clusterThread));
                clusterThread.setThread(t);
                while (!clusterThread.isReady) {
                    try {
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    } catch(...) {
                        throw  exception::IException("ClusterService::start", "ClusterService can not be started");
                    }
                }
                //TODO init membershipEvent
                active = true;
            }


            void ClusterService::stop() {
                active = false;
                clusterThread.stop();
            }

            boost::shared_future<serialization::Data> ClusterService::send(const impl::PortableRequest &request) {
                connection::Connection *connection = getOrConnect(NULL);
                return doSend(request, NULL, *connection);
            }

            boost::shared_future<serialization::Data> ClusterService::send(const impl::PortableRequest &request, const Address &address) {
                connection::Connection *connection = getOrConnect(&address);
                return doSend(request, NULL, *connection);
            }

            boost::shared_future<serialization::Data> ClusterService::send(const impl::PortableRequest &request, connection::Connection &connection) {
                return doSend(request, NULL, connection);
            }

            boost::shared_future<serialization::Data> ClusterService::send(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler) {
                connection::Connection *connection = getOrConnect(NULL);
                return doSend(request, eventHandler, *connection);
            }

            boost::shared_future<serialization::Data> ClusterService::send(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, const Address &address) {
                connection::Connection *connection = getOrConnect(&address);
                return doSend(request, eventHandler, *connection);
            }

            boost::shared_future<serialization::Data> ClusterService::doSend(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, connection::Connection &connection) {
                util::CallPromise *promise = new util::CallPromise(*this);
                promise->setRequest(&request);
                promise->setEventHandler(eventHandler);
                _send(promise, connection);
                return promise->getFuture();
            }


            void ClusterService::_send(util::CallPromise *promise, connection::Connection &connection) {
                connection.registerCall(promise);
                serialization::Data data = serializationService.toData<impl::PortableRequest>(&(promise->getRequest()));
                if (!connection.write(data)) {
                    promise->targetIsNotAlive(connection.getRemoteEndpoint());
                    connection.removeConnectionCalls();//TODO connectionCallMap.remove(connection);
                }
            }

            void ClusterService::registerListener(const std::string &uuid, int callId) {
                registrationAliasMap.put(uuid, &uuid);
                registrationIdMap.put(uuid, &callId);
            }

            void ClusterService::reRegisterListener(const std::string &uuid, const std::string &alias, int callId) {
                const string *oldAlias = registrationAliasMap.put(uuid, &alias);
                if (oldAlias != NULL) {
                    registrationIdMap.remove(*oldAlias);
                    registrationIdMap.put(alias, &callId);
                }
            }

            bool ClusterService::deRegisterListener(const std::string &uuid) {
                const std::string *alias = registrationAliasMap.remove(uuid);
                if (alias != NULL) {
                    int *callId = registrationIdMap.remove(*alias);
                    connectionManager.removeEventHandler(*callId);
                    return true;
                }
                return false;
            }

            void ClusterService::resend(util::CallPromise *promise) {
                connection::Connection *connection = getOrConnect(NULL);
                connection->reRegisterCall(promise);
                serialization::Data data = serializationService.toData<impl::PortableRequest>(&(promise->getRequest()));
                connection->write(data);
            }

            std::auto_ptr<Address> ClusterService::getMasterAddress() {
                vector<connection::Member> list = getMemberList();
                if (list.empty()) {
                    return std::auto_ptr<Address>(NULL);
                }
                return std::auto_ptr<Address>(new Address(list[0].getAddress()));
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                listeners.insert(listener);
            };

            bool ClusterService::removeMembershipListener(MembershipListener *listener) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                bool b = listeners.erase(listener) == 1;
                return b;
            };


            bool ClusterService::isMemberExists(Address const &address) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                return members.count(address) > 0;;
            };

            connection::Member ClusterService::getMember(Address &address) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                return members[address];

            }

            connection::Member ClusterService::getMember(const std::string &uuid) {
                vector<connection::Member> list = getMemberList();
                for (vector<connection::Member>::iterator it = list.begin(); it != list.end(); ++it) {
                    if (uuid.compare(it->getUuid())) {
                        return *it;
                    }
                }
                return connection::Member();
            };


            bool ClusterService::isRedoOperation() const {
                return redoOperation;
            }

            std::vector<connection::Member>  ClusterService::getMemberList() {
                typedef std::map<Address, connection::Member, addressComparator> MemberMap;
                std::vector<connection::Member> v;
                boost::lock_guard<boost::mutex> guard(membersLock);
                MemberMap::const_iterator it;
                for (it = members.begin(); it != members.end(); it++) {
                    v.push_back(it->second);
                }
                return v;
            };

            void ClusterService::handlePacket(connection::Connection &connection, serialization::Data &data) {

                boost::shared_ptr<connection::ClientResponse> response = serializationService.toObject<connection::ClientResponse>(data);
                if (response->isEvent()) {
                    util::CallPromise *promise = connection.getEventHandler(response->getCallId());
                    if (promise != NULL) {
                        promise->getEventHandler()->handle(response->getData());
                        return;
                    }
                    return;
                }
                {
//                    boost::lock_guard<boost::mutex> l(connectionLock);TODO
                    util::CallPromise *promise = connection.deRegisterCall(response->getCallId());
                    if (response->isException()) {
                        promise->setException(response->getException());
                    } else {

                        if (promise->getEventHandler() != NULL) {
                            //TODO may require lock here
                            boost::shared_ptr<std::string> alias = serializationService.toObject<std::string>(response->getData());
                            boost::shared_ptr<std::string> uuid = serializationService.toObject<std::string>(promise->getFuture().get());
                            int callId = promise->getRequest().callId;
                            reRegisterListener(*uuid, *alias, callId);
                            return;
                        }
                        promise->setResponse(response->getData());
                    }

                }

            }


            connection::Connection *ClusterService::getOrConnect(const Address *target) {
                int count = 0;
                exception::IOException lastError("", "");
                while (count < RETRY_COUNT) {
                    try {
                        if (target == NULL || !isMemberExists(*target)) {
                            return connectionManager.getRandomConnection();
                        } else {
                            return connectionManager.getOrConnect(*target);
                        }
                    } catch (exception::IOException &e) {
                        lastError = e;
                    }
                    target = NULL;
                    count++;
                }
                throw lastError;
            }

            //--------- Used by CLUSTER LISTENER THREAD ------------

            connection::Connection *ClusterService::connectToOne(const std::vector<Address> &socketAddresses) {
                active = false;
                const int connectionAttemptLimit = clientConfig.getConnectionAttemptLimit();
                int attempt = 0;
                std::exception lastError;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            connection::Connection *pConnection = connectionManager.ownerConnection(*it);
                            active = true;
                            return pConnection;
                        } catch (exception::IOException &e) {
                            lastError = e;
                            std::cerr << "IO error  during initial connection..\n" << e.what() << std::endl;
                        } catch (exception::ServerException &e) {
                            lastError = e;
                            std::cerr << "IO error from server during initial connection..\n" << e.what() << std::endl;

                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = clientConfig.getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    using namespace std;
                    std::cerr << "Unable to get alive cluster connection, try in " << max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

                    if (remainingTime > 0) {
                        boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
                    }
                }
                throw  exception::IException("ClusterService", "Unable to connect to any address in the config!" + std::string(lastError.what()));
            };


            void ClusterService::fireMembershipEvent(connection::MembershipEvent &event) {
                boost::lock_guard<boost::mutex> guard(listenerLock);
                for (std::set<MembershipListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    if (event.getEventType() == connection::MembershipEvent::MEMBER_ADDED) {
                        (*it)->memberAdded(event);
                    } else {
                        (*it)->memberRemoved(event);
                    }
                }
            };

            void ClusterService::setMembers(const std::map<Address, connection::Member, addressComparator > &map) {
                boost::lock_guard<boost::mutex> guard(membersLock);
                members = map;
            }
        }
    }
}
