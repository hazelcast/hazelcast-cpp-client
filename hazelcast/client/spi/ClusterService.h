//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "../Address.h"
#include "../connection/Connection.h"
#include "../connection/ClusterListenerThread.h"
#include "../connection/ConnectionManager.h"
#include "../protocol/HazelcastServerError.h"
#include "../serialization/SerializationService.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace spi {

            class ClusterService {
            public:
                ClusterService(hazelcast::client::HazelcastClient& client);

                void start();

                template<typename send_type, typename recv_type>
                void sendAndReceive(send_type& object, recv_type& response) {
                    hazelcast::client::connection::Connection& conn = getConnectionManager().getRandomConnection();
                    return sendAndReceive(conn, object, response);
                }

                template<typename send_type, typename recv_type>
                void sendAndReceive(const Address& address, send_type& object, recv_type& response) {
                    hazelcast::client::connection::Connection& conn = getConnectionManager().getConnection(address);
                    return sendAndReceive(conn, object, response);
                }

                template<typename send_type, typename recv_type>
                void sendAndReceive(hazelcast::client::connection::Connection& connection, send_type& object, recv_type& response) {
                    using namespace hazelcast::client::serialization;
                    SerializationService& serializationService = getSerializationService();
                    Data request = serializationService.toData(object);
                    connection.write(request);
                    Data responseData;
                    connection.read(responseData);
                    if (responseData.isServerError()) {
                        throw  serializationService.toObject<hazelcast::client::protocol::HazelcastServerError>(responseData);
                    } else {
                        response = serializationService.toObject<recv_type>(responseData);

                    }
                };


            private:
                hazelcast::client::connection::ClusterListenerThread clusterThread;
                hazelcast::client::HazelcastClient& hazelcastClient;

                hazelcast::client::connection::Connection *connectToOne(const std::vector<hazelcast::client::Address>& socketAddresses);

                hazelcast::client::connection::ConnectionManager& getConnectionManager();

                hazelcast::client::serialization::SerializationService& getSerializationService();

                hazelcast::client::ClientConfig & getClientConfig();
            };

        }
    }
}
#endif //HAZELCAST_CLUSTER_SERVICE
