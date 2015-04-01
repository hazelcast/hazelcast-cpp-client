//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_HazelcastServerFactory
#define HAZELCAST_HazelcastServerFactory

#include "hazelcast/client/Address.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastServer;

            class HazelcastServerFactory {
                friend class HazelcastServer;

                enum {
                    OK = 5678,
                    END = 1,
                    START = 2,
                    SHUTDOWN = 3,
                    SHUTDOWN_ALL = 4
                };
            public:
                HazelcastServerFactory(const char* hostAddress);

                const std::string& getServerAddress() const;

                void shutdownAll();

                int getInstanceId();

                ~HazelcastServerFactory();

            private:
                Address address;
                Socket socket;
                connection::OutputSocketStream outputSocketStream;
                connection::InputSocketStream inputSocketStream;

                void shutdownInstance(int id);
            };
        }
    }
}

#endif //HAZELCAST_HazelcastServerFactory

