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
#import "mintomic_msvc.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace spi {

            class ClusterService {
            public:
                ClusterService(HazelcastClient& client);

                void start();

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(send_type& object) {
                    recv_type response;
                    connection::Connection& conn = getConnectionManager().getRandomConnection();
                    response = sendAndReceive<recv_type>(conn, object);
                    getConnectionManager().releaseConnection(&conn);
                    return response;
                }

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(const Address& address, send_type& object) {
                    connection::Connection& conn = getConnectionManager().getConnection(address);
                    recv_type response = sendAndReceive<recv_type>(conn, object);
                    getConnectionManager().releaseConnection(&conn);
                    return response;
                }

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(connection::Connection& connection, send_type& object) {
                    serialization::SerializationService& serializationService = getSerializationService();
                    serialization::Data request = serializationService.toData(object);
                    connection.write(request);
                    serialization::Data responseData = connection.read(serializationService.getSerializationContext());
                    if (responseData.isServerError()) {
                        protocol::HazelcastServerError x = serializationService.toObject<protocol::HazelcastServerError>(responseData);
                        std::cerr << x.what() << std::endl;
                        throw  x;
                    }
                    recv_type response = serializationService.toObject<recv_type>(responseData);

                    return response;

                };

                Address getMasterAddress();

                void addMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                std::vector<connection::Member> getMemberList();

                friend class connection::ClusterListenerThread;

            private:

                connection::ClusterListenerThread clusterThread;
                HazelcastClient& hazelcastClient;
                util::AtomicPointer< std::map<Address, connection::Member > > membersRef;
                util::ConcurrentMap< MembershipListener *, bool> listeners;

                connection::Connection *connectToOne(const std::vector<Address>& socketAddresses);


                connection::ConnectionManager& getConnectionManager();

                serialization::SerializationService& getSerializationService();

                ClientConfig & getClientConfig();
            };

        }
    }
}
#endif //HAZELCAST_CLUSTER_SERVICE
