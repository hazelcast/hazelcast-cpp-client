//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_DistributedObjectListenerService
#define HAZELCAST_DistributedObjectListenerService

#include <boost/thread/mutex.hpp>
#include <set>

namespace hazelcast {
    namespace client {
        class DistributedObjectListener;

        namespace spi {


            class DistributedObjectListenerService {
            public:

                void addDistributedObject(const std::string& name);

                void removeDistributedObject(const std::string& name);

                void addDistributedObjectListener(DistributedObjectListener *listener);

                bool removeDistributedObjectListener(DistributedObjectListener *listener);

            private:

                std::set<DistributedObjectListener *> listeners;
                boost::mutex listenerLock;
            };
        }
    }
}

#endif //HAZELCAST_DistributedObjectListenerService
