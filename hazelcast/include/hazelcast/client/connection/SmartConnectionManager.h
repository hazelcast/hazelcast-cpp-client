//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SmartConnectionManager
#define HAZELCAST_SmartConnectionManager

#include "HeartBeatChecker.h"
#include "ConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class SmartConnectionManager : public ConnectionManager {
            public:
                SmartConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService&, ClientConfig&);

            };
        }
    }
}

#endif //HAZELCAST_SmartConnectionManager
