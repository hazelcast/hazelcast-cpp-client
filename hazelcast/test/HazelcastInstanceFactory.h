//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_HazelcastInstanceFactory
#define HAZELCAST_HazelcastInstanceFactory

#include "hazelcast/client/Address.h"
#include "hazelcast/client/connection/Socket.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/InputSocketStream.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class HazelcastInstance;

            class HAZELCAST_API HazelcastInstanceFactory {
                friend class HazelcastInstance;

                enum {
                    OK = 5678,
                    END = 1,
                    START = 2,
                    SHUTDOWN = 3,
                    SHUTDOWN_ALL = 4
                };
            public:
                HazelcastInstanceFactory();

                void shutdownAll();

                int getInstanceId();

                ~HazelcastInstanceFactory();

            private:
                Address address;
                connection::Socket socket;
                serialization::OutputSocketStream outputSocketStream;
                serialization::InputSocketStream inputSocketStream;

                void shutdownInstance(int id);
            };
        }
    }
}

#endif //HAZELCAST_HazelcastInstanceFactory
