//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "../Address.h"
#include "../HazelcastClient.h"
#include "../protocol/Credentials.h"
#include "../connection/Connection.h"
#include "../connection/ConnectionManager.h"
#include "BufferedDataInput.h"
#include "HazelcastServerError.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        class protocol::Credentials;

        namespace spi {

            class ClusterService {
            public:
                ClusterService(hazelcast::client::HazelcastClient& client);

                template<typename send_type, typename recv_type>
                void sendAndReceive(send_type& object, recv_type& response) {
                    hazelcast::client::connection::Connection& conn = getConnectionManager().getRandomConnection();
                    return sendAndReceive(conn, object, response);
                }

                template<typename send_type, typename recv_type>
                void sendAndReceive(Address address, send_type& object, recv_type& response) {
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

                hazelcast::client::connection::ConnectionManager& getConnectionManager();

                hazelcast::client::serialization::SerializationService& getSerializationService();

            private:
                HazelcastClient& client;
                hazelcast::client::protocol::Credentials& credentials;
            };

        }
    }
}
#endif //HAZELCAST_CLUSTER_SERVICE
