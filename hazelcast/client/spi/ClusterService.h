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
#include "../../util/AtomicPointer.h"
#include "PartitionService.h"
#include "../spi/ClientContext.h"
#include "../../util/Lock.h"
#include <set>

namespace hazelcast {
    namespace client {

        namespace spi {

            class ClusterService {
            public:
                ClusterService(ClientContext& clientContext);

                void start();

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(send_type& object) {
                    recv_type response;
                    connection::Connection *conn = getConnectionManager().getRandomConnection();
                    response = sendAndReceive<recv_type>(conn, object);
                    getConnectionManager().releaseConnection(conn);
                    return response;
                };

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(const Address& address, send_type& object) {
                    connection::Connection *conn = getConnectionManager().getConnection(address);
                    recv_type response = sendAndReceive<recv_type>(conn, object);
                    getConnectionManager().releaseConnection(conn);
                    return response;
                };

                template< typename recv_type, typename send_type>
                recv_type sendAndReceive(connection::Connection *connection, send_type& object) {
                    try {
                        serialization::SerializationService& serializationService = getSerializationService();
                        serialization::Data request = serializationService.toData(object);
                        connection->write(request);
                        serialization::Data responseData = connection->read(serializationService.getSerializationContext());
                        return serializationService.toObject<recv_type>(responseData);
                    } catch(HazelcastException hazelcastException){
                        clientContext.getPartitionService().refreshPartitions();
                        if (redoOperation /*|| dynamic_cast<const impl::RetryableRequest *>(&object) != NULL*/) {//TODO global isRetryable(const T& a) function solves
                            return sendAndReceive<recv_type>(object);
                        }
                        throw hazelcastException;
                    }
                };

                Address getMasterAddress();

                void addMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                bool isMemberExists(const Address& address);

                std::vector<connection::Member> getMemberList();

                friend class connection::ClusterListenerThread;

            private:

                connection::ClusterListenerThread clusterThread;
                protocol::Credentials& credentials;
                ClientContext& clientContext;
                util::AtomicPointer< std::map<Address, connection::Member > > membersRef;
                std::set< MembershipListener *> listeners;
                util::Lock listenerLock;
                const bool redoOperation;

                void fireMembershipEvent(connection::MembershipEvent& membershipEvent);

                connection::Connection *getConnection(const Address& address);

                connection::Connection *getRandomConnection();

                connection::Connection *connectToOne(const std::vector<Address>& socketAddresses);

                connection::ConnectionManager& getConnectionManager();

                serialization::SerializationService& getSerializationService();

                ClientConfig & getClientConfig();
            };

        }
    }
}

#endif //HAZELCAST_CLUSTER_SERVICE
