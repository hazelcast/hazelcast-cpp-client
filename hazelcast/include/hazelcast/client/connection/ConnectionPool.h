//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
#ifndef HAZELCAST_QUEUE_BASED_OBJECT_POOL
#define HAZELCAST_QUEUE_BASED_OBJECT_POOL

#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/Address.h"
#include <boost/atomic.hpp>

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }

        namespace connection {
            class ConnectionManager;

            class Connection;

            class HAZELCAST_API ConnectionPool {
            public:
                ConnectionPool(const Address& address, hazelcast::client::serialization::SerializationService&, ConnectionManager &connectionManager);

                ~ConnectionPool();

                Connection *take();

                void release(Connection *t);

                Address address;
            private:
                boost::atomic<bool> active;
                hazelcast::util::ConcurrentQueue<Connection *> queue;
                hazelcast::client::serialization::SerializationService& serializationService;
                ConnectionManager &connectionManager;

            };
        }
    }
}

#endif //__QueueBasedObjectPool_H_
