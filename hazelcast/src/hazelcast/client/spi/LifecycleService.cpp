//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include <boost/thread/lock_guard.hpp>

namespace hazelcast {
    namespace client {

        namespace spi {

            LifecycleService::LifecycleService(HazelcastClient & hazelcastClient, ClientConfig& config)
            :hazelcastClient(hazelcastClient)
            , active(false) {
                std::set<spi::EventListener *> listeners = config.getListeners();
                if (!listeners.empty()) {
                    for (std::set<spi::EventListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                        LifecycleListener *listener = dynamic_cast<LifecycleListener *>(*it);
                        if (listener) {
                            addLifecycleListener(listener);
                        }
                    }
                }
                fireLifecycleEvent(LifecycleEvent::STARTING);
            };

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                listeners.insert(lifecycleListener);
            };

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            };

            void LifecycleService::fireLifecycleEvent(LifecycleEvent lifecycleEvent) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                };
            };

            void LifecycleService::setStarted() {
                active = true;
                fireLifecycleEvent(LifecycleEvent::STARTED);
            };

            bool LifecycleService::isRunning() {
                return active;
            };

            void LifecycleService::shutdown() {
                active = false;
                boost::lock_guard<boost::mutex> lg(lifecycleLock);
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                hazelcastClient.shutdown();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
            };


        }
    }
}