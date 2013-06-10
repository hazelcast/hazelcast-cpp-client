//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_QUEUE_BASED_OBJECT_POOL
#define HAZELCAST_QUEUE_BASED_OBJECT_POOL

#include "../../util/ConcurrentQueue.h"
#include "../HazelcastException.h"
#include "../../util/Lock.h"
#include "../Address.h"

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }

        namespace connection {
            class ConnectionManager;

            class Connection;

            class ConnectionPool {
            public:
                ConnectionPool(const Address& address, hazelcast::client::serialization::SerializationService&, ConnectionManager &connectionManager);

                ~ConnectionPool();

                Connection *take();

                void release(Connection *t);

                void destroy();

            private:
                volatile bool active;
                hazelcast::util::ConcurrentQueue<Connection *> queue;
                hazelcast::client::serialization::SerializationService& serializationService;
                ConnectionManager &connectionManager;
                Address address;

            };
        }
    }
}

#endif //__QueueBasedObjectPool_H_
