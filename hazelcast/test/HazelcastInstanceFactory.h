//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_HazelcastInstanceFactory
#define HAZELCAST_HazelcastInstanceFactory

#include "Address.h"
#include "Socket.h"
#include "OutputSocketStream.h"
#include "InputSocketStream.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class HazelcastInstance;

            class HazelcastInstanceFactory {
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

                HazelcastInstance newHazelcastInstance();

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
