//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "DistributedObjectListenerService.h"
#include "boost/thread/lock_guard.hpp"
#include "DistributedObjectEvent.h"
#include "DistributedObjectListener.h"


namespace hazelcast {
    namespace client {
        namespace spi {


            void DistributedObjectListenerService::addDistributedObject(const std::string& name) {
                boost::lock_guard<boost::mutex> lg(listenerLock);

                DistributedObjectEvent event(DistributedObjectEvent::CREATED, name);
                for (std::set<DistributedObjectListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                        (*it)->distributedObjectCreated(event);
                };
            }

            void DistributedObjectListenerService::removeDistributedObject(const std::string& name) {
                boost::lock_guard<boost::mutex> lg(listenerLock);

                DistributedObjectEvent event(DistributedObjectEvent::DESTROYED, name);
                for (std::set<DistributedObjectListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->distributedObjectDestroyed(event);
                };

            }

            void DistributedObjectListenerService::addDistributedObjectListener(DistributedObjectListener *listener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                listeners.insert(listener);
            }

            bool DistributedObjectListenerService::removeDistributedObjectListener(DistributedObjectListener *listener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                return listeners.erase(listener) == 1;
            }

        }
    }
}