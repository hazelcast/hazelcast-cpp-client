//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "ClientContext.h"
#include "PartitionService.h"
#include "ResponseStream.h"
#include "../connection/Connection.h"
#include "../connection/ClusterListenerThread.h"
#include "../connection/ConnectionManager.h"
#include "ServerException.h"
#include "../Address.h"
#include "../../util/AtomicPointer.h"
#include "../../util/Lock.h"
#include "../serialization/SerializationService.h"
#include <set>

namespace hazelcast {
    namespace client {

        namespace spi {

            class ClusterService {
            public:
                ClusterService(spi::PartitionService&, spi::LifecycleService&, connection::ConnectionManager&, serialization::SerializationService&, ClientConfig &);

                void start();

                template< typename Response, typename Request>
                Response sendAndReceive(const Request& object) {
                    Response response;
                    connection::Connection *conn = connectionManager.getRandomConnection();
                    response = sendAndReceive<Response>(conn, object);
                    connectionManager.releaseConnection(conn);
                    return response;
                };

                template< typename Response, typename Request>
                Response sendAndReceive(const Address& address, const Request& object) {
                    connection::Connection *conn = connectionManager.getConnection(address);
                    Response response = sendAndReceive<Response>(conn, object);
                    connectionManager.releaseConnection(conn);
                    return response;
                };


                template< typename Request, typename ResponseHandler>
                void sendAndHandle(const Address& address, const Request& obj, const ResponseHandler&  handler) {
                    connection::Connection *conn = getConnection(address);
                    sendAndHandle(conn, obj, handler);
                };

                template< typename Request, typename ResponseHandler>
                void sendAndHandle(const Request& obj, const ResponseHandler& handler) {
                    connection::Connection *conn = getRandomConnection();
                    sendAndHandle(conn, obj, handler);
                };

                Address getMasterAddress();

                bool isMemberListEmpty();

                void addMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                bool isMemberExists(const Address& address);

                connection::Member getMember(const std::string& uuid);

                std::vector<connection::Member> getMemberList();

                friend class connection::ClusterListenerThread;

            private:
                connection::ConnectionManager& connectionManager;
                serialization::SerializationService& serializationService;
                ClientConfig & clientConfig;
                spi::LifecycleService& lifecycleService;
                spi::PartitionService& partitionService;

                connection::ClusterListenerThread clusterThread;
                protocol::Credentials& credentials;
                util::AtomicPointer< std::map<Address, connection::Member > > membersRef;
                std::set< MembershipListener *> listeners;
                util::Lock listenerLock;
                const bool redoOperation;


                template< typename Response, typename Request>
                Response sendAndReceive(connection::Connection *connection, const Request& object) {
                    try {
                        serialization::Data request = serializationService.toData<Request>(&object);
                        connection->write(request);
                        serialization::Data responseData = connection->read(serializationService.getSerializationContext());
                        return serializationService.toObject<Response>(responseData);
                    } catch(exception::IException& e){
                        partitionService.refreshPartitions();
                        if (redoOperation /*|| dynamic_cast<const impl::RetryableRequest *>(&object) != NULL*/) {//TODO global isRetryable(const T& a) function solves
                            return sendAndReceive<Response>(object);
                        }
                        throw e;
                    }
                };

                template< typename Request, typename ResponseHandler>
                void sendAndHandle(connection::Connection *conn, const Request& obj, const ResponseHandler&  handler) {
                    ResponseStream stream(serializationService, *conn);
                    try {
                        serialization::Data request = serializationService.toData<Request>(&obj);
                        conn->write(request);
                    } catch (exception::IOException& e){
                        partitionService.refreshPartitions();
                        if (redoOperation /*obj instanceof RetryableRequest*/) {
                            sendAndHandle(obj, handler);
                            return;
                        }
                        throw exception::IException(e);
                    }

                    try {
                        handler.handle(stream);
                    } catch (exception::IException& e) {
                        stream.end();
                        throw e;//ClientException(e);
                    }
                    stream.end();
                }


                void fireMembershipEvent(connection::MembershipEvent& membershipEvent);

                connection::Connection *getConnection(const Address& address);

                connection::Connection *getRandomConnection();

                connection::Connection *connectToOne(const std::vector<Address>& socketAddresses);

            };

        }
    }
}

#endif //HAZELCAST_CLUSTER_SERVICE
