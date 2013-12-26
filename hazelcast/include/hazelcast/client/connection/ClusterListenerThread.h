////
//// Created by sancar koyunlu on 5/23/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
//#define HAZELCAST_CLUSTER_LISTENER_THREAD
//
//#include "hazelcast/client/connection/Member.h"
//#include <boost/atomic.hpp>
//
//namespace hazelcast {
//    namespace client {
//
//        class ClientConfig;
//
//        namespace spi {
//            class ClusterService;
//
//            class LifecycleService;
//        }
//
//        namespace connection {
//
//            class Connection;
//
//            class ConnectionManager;
//
//            class MembershipEvent;
//
//            class HAZELCAST_API ClusterListenerThread {
//            public:
//                ClusterListenerThread(ConnectionManager &, ClientConfig &clientConfig, spi::ClusterService &, spi::LifecycleService &, serialization::SerializationService &);
//
//                void setInitialConnection(connection::Connection *);
//
//                void setThread(boost::thread *);
//
//                void run();
//
//                void stop();
//
//                boost::atomic<bool> isReady;
//            private:
//                ConnectionManager &connectionManager;
//                spi::ClusterService &clusterService;
//                spi::LifecycleService &lifecycleService;
//                serialization::SerializationService &serializationService;
//
//                std::auto_ptr<boost::thread> clusterListenerThread;
//
//                std::auto_ptr<Connection> conn;
//                boost::atomic<bool> deletingConnection;
//                std::vector<Member> members;
//                ClientConfig &clientConfig;
//
//                Connection *pickConnection();
//
//                void loadInitialMemberList();
//
//                void listenMembershipEvents();
//
//                void updateMembersRef();
//
//                std::vector<Address> getClusterAddresses() const;
//
//                std::vector<Address> getConfigAddresses() const;
//            };
//        }
//    }
//}
//
//#endif //HAZELCAST_CLUSTER_LISTENER_THREAD
