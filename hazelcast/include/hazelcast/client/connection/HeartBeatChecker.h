//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_HEARTBEAT_CHECKER
#define HAZELCAST_HEARTBEAT_CHECKER

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/protocol/PingRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;
        }
        namespace connection {

            class Connection;

            class HAZELCAST_API HeartBeatChecker {

            public:
                HeartBeatChecker(int timeout, serialization::SerializationService& serializationService);

                bool checkHeartBeat(Connection& connection);

                void run(Connection *connection);

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
