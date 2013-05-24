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
            ClusterService::ClusterService(hazelcast::client::HazelcastClient & hazelcastClient)
            : hazelcastClient(hazelcastClient) {

            }

            void ClusterService::start() {
                hazelcast::client::serialization::ClassDefinitionBuilder cd(-3, 3);
                hazelcast::client::serialization::ClassDefinition *ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
                hazelcastClient.getSerializationService().getSerializationContext()->registerClassDefinition(ptr);

                hazelcast::client::connection::Connection *f = connectToOne(getClientConfig().getAddresses());
//                try {
//                    final Connection connection = f.get(30, TimeUnit.SECONDS);
                clusterThread.setInitialConnection(f);
//                } catch (Exception e) {
//                    throw new ClientException(e);
//                }
                clusterThread.start();
//
//                // TODO: replace with a better wait-notify
//                while (membersRef.get() == null) {
//                    try {
//                        Thread.sleep(100);
//                    } catch (InterruptedException e) {
//                        throw new ClientException(e);
//                    }
//                }
            }

            hazelcast::client::connection::Connection *ClusterService::connectToOne(const std::vector<hazelcast::client::Address>& socketAddresses) {
                const int connectionAttemptLimit = getClientConfig().getConnectionAttemptLimit();
//            final ManagerAuthenticator authenticator = new ManagerAuthenticator();
                int attempt = 0;
                while (true) {
                    time_t tryStartTime = std::time(NULL);
                    std::vector<Address>::const_iterator it;
                    for (it = socketAddresses.begin(); it != socketAddresses.end(); it++) {
                        try {
                            std::cout << "Trying to connect: " + (*it).getAddress() + ":" + (*it).getPort() << std::endl;
                            return getConnectionManager().newConnection((*it));
                        } catch (hazelcast::client::HazelcastException& ignored) {
                        }
                    }
                    if (attempt++ >= connectionAttemptLimit) {
                        break;
                    }
                    const double remainingTime = getClientConfig().getAttemptPeriod() - std::difftime(std::time(NULL), tryStartTime);
                    std::cerr << "Unable to get alive cluster connection, try in " << std::max(0.0, remainingTime)
                            << " ms later, attempt " << attempt << " of " << connectionAttemptLimit << "." << std::endl;

//
                    if (remainingTime > 0) {
//                    try {
//                        Thread.sleep(remainingTime);
//                    } catch (InterruptedException e) {
//                        break;
//                    }
                    }
                }
                throw  hazelcast::client::HazelcastException("Unable to connect to any address in the config!");
            };

            hazelcast::client::connection::ConnectionManager& ClusterService::getConnectionManager() {
                return hazelcastClient.getConnectionManager();
            };

            hazelcast::client::serialization::SerializationService & ClusterService::getSerializationService() {
                return hazelcastClient.getSerializationService();
            };

            hazelcast::client::ClientConfig & ClusterService::getClientConfig() {
                return hazelcastClient.getClientConfig();
            };


        }
    }
}
