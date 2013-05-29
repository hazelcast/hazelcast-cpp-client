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
#include "../../util/AtomicPointer.h"
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
                    sendAndReceive(conn, object, response);
                    getConnectionManager().releaseConnection(&conn);
                }

                template<typename send_type, typename recv_type>
                void sendAndReceive(const Address& address, send_type& object, recv_type& response) {
                    hazelcast::client::connection::Connection& conn = getConnectionManager().getConnection(address);
                    sendAndReceive(conn, object, response);
                    getConnectionManager().releaseConnection(&conn);
                }

                template<typename send_type, typename recv_type>
                void sendAndReceive(hazelcast::client::connection::Connection& connection, send_type& object, recv_type& response) {
                    using namespace hazelcast::client::serialization;
                    SerializationService& serializationService = getSerializationService();
                    Data request;
                    serializationService.toData(object, request);
                    connection.write(request);
                    Data responseData;
                    responseData.setSerializationContext(serializationService.getSerializationContext());
                    connection.read(responseData);
                    if (responseData.isServerError()) {
                        hazelcast::client::protocol::HazelcastServerError x;
                        serializationService.toObject(responseData, x);
                        std::cerr << x.what() << std::endl;
                        throw  x;
                    } else {
                        serializationService.toObject(responseData, response);
                    }

                };


                friend class hazelcast::client::connection::ClusterListenerThread;

            private:

                hazelcast::client::connection::ClusterListenerThread clusterThread;
                hazelcast::client::HazelcastClient& hazelcastClient;
                hazelcast::util::AtomicPointer< std::map<std::string, hazelcast::client::connection::Member > > membersRef;

                hazelcast::client::connection::Connection *connectToOne(const std::vector<hazelcast::client::Address>& socketAddresses);


                hazelcast::client::connection::ConnectionManager& getConnectionManager();

                hazelcast::client::serialization::SerializationService& getSerializationService();

                hazelcast::client::ClientConfig & getClientConfig();
            };

        }
    }
}
#endif //HAZELCAST_CLUSTER_SERVICE
