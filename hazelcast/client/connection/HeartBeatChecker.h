//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_HEARTBEAT_CHECKER
#define HAZELCAST_HEARTBEAT_CHECKER

#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;
        }
        namespace connection {

            class Connection;

            class HeartBeatChecker {
            public:
                HeartBeatChecker(int timeout, hazelcast::client::serialization::SerializationService& serializationService);

                bool checkHeartBeat(const Connection& connection);

            private:
                int timeout;
                hazelcast::client::serialization::Data ping;
                hazelcast::client::serialization::SerializationService& serializationService;
            };

        }
    }
}
#endif //HAZELCAST_HEARTBEAT_CHECKER
