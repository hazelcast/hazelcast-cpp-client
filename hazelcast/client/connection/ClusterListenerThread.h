//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
#define HAZELCAST_CLUSTER_LISTENER_THREAD

namespace hazelcast {
    namespace client {
        namespace connection {

            class Connection;

            class ClusterListenerThread {
            public:
                ClusterListenerThread();

                void setInitialConnection(hazelcast::client::connection::Connection *);

                void start();

            private:
                Connection *connection;
            };
        }
    }
}

#endif //HAZELCAST_CLUSTER_LISTENER_THREAD
