//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_HEARTBEAT_CHECKER
#define HAZELCAST_HEARTBEAT_CHECKER

#include "../serialization/Data.h"
#include "../protocol/PingRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;
        }
        namespace connection {

            class Connection;

            class HeartBeatChecker {

            public:
                HeartBeatChecker(int timeout, serialization::SerializationService& serializationService);

                bool checkHeartBeat(Connection& connection);

                void run(Connection* connection);

            private:

                int timeout;
                protocol::PingRequest pingRequest;
                serialization::Data ping;
                serialization::SerializationService& serializationService;
            };

        }
    }
}
#endif //HAZELCAST_HEARTBEAT_CHECKER
