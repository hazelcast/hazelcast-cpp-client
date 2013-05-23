//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_CLIENT_CONTEXT
#define HAZELCAST_CLIENT_CONTEXT

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }
        class HazelcastClient;

        class ClientConfig;


        namespace spi {
            class InvocationService;

            class ClusterService;


            class ClientContext {
            public:

                ClientContext(hazelcast::client::HazelcastClient& hazelcastClient);

                hazelcast::client::serialization::SerializationService& getSerializationService();

                ClusterService& getClusterService();

                InvocationService& getInvocationService();

                hazelcast::client::ClientConfig& getClientConfig();


            private:
                hazelcast::client::HazelcastClient& hazelcastClient;
            };
        }
    }
}

#endif