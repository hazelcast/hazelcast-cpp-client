//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ClusterService.h"
#include "../ClientConfig.h"
#include "../HazelcastClient.h"
#include "../serialization/ClassDefinitionBuilder.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(HazelcastClient & hazelcastClient)
            : hazelcastClient(hazelcastClient)
            , clusterThread(hazelcastClient.getConnectionManager(), hazelcastClient.getClientConfig(), *this) {

            }

            void ClusterService::start() {
                serialization::ClassDefinitionBuilder cd(-3, 3);
                serialization::ClassDefinition *ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
                hazelcastClient.getSerializationService().getSerializationContext().registerClassDefinition(ptr);

                connection::Connection *connection = connectToOne(getClientConfig().getAddresses());
                clusterThread.setInitialConnection(connection);
                clusterThread.start();
                while (membersRef.get() == NULL) {
                    try {
                        sleep(1);
                    } catch (void* ) {
                        throw  HazelcastException("ClusterService::start");
                    }
                }
            }

            connection::Connection *ClusterService::connectToOne(const std::vector<Address>& socketAddresses) {
                const int connectionAttemptLimit = getClientConfig().getConnectionAttemptLimit();
                int attempt = 0;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            std::cout << "Trying to connect: " + (*it).getHost() + ":" + util::to_string((*it).getPort()) << std::endl;
                            return getConnectionManager().newConnection((*it));
                        } catch (HazelcastException& ignored) {
                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = getClientConfig().getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    std::cerr << "Unable to get alive cluster connection, try in " << std::max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

                    if (remainingTime > 0) {
                        sleep(remainingTime);
                    }
                }
                throw  HazelcastException("Unable to connect to any address in the config!");
            };


            Address ClusterService::getMasterAddress() {
                vector<connection::Member> list = getMemberList();
                return list[0].getAddress();
            }

            void ClusterService::addMembershipListener(MembershipListener *listener) {
                static bool dummyBool = true;
                listeners.put(listener, &dummyBool);
            };

            bool ClusterService::removeMembershipListener(MembershipListener *listener) {
                return listeners.remove(listener) != NULL;
            };

            vector<connection::Member>  ClusterService::getMemberList() {
                return util::values(membersRef.get());
            };

            connection::ConnectionManager& ClusterService::getConnectionManager() {
                return hazelcastClient.getConnectionManager();
            };

            serialization::SerializationService & ClusterService::getSerializationService() {
                return hazelcastClient.getSerializationService();
            };

            ClientConfig & ClusterService::getClientConfig() {
                return hazelcastClient.getClientConfig();
            };


        }
    }
}
