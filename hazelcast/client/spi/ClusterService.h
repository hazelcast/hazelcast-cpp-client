//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "Credentials.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        class protocol::Credentials;

        namespace spi {

            class ClusterService {
            public:
                ClusterService(hazelcast::client::HazelcastClient& client);

            private:
                HazelcastClient& client;
                hazelcast::client::protocol::Credentials& credentials;
            };

        }
    }
}
#endif //HAZELCAST_CLUSTER_SERVICE
