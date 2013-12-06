//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SmartConnectionManager
#define HAZELCAST_SmartConnectionManager

#include "hazelcast/client/connection/HeartBeatChecker.h"
#include "hazelcast/client/connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class HAZELCAST_API SmartConnectionManager : public ConnectionManager {
            public:
                SmartConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService&, ClientConfig&);

            };
        }
    }
}

#endif //HAZELCAST_SmartConnectionManager
