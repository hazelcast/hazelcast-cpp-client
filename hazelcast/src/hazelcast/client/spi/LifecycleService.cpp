//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            LifecycleService::LifecycleService(ClientContext &clientContext, const ClientConfig &clientConfig)
            :clientContext(clientContext)
            , active(false) {
                std::set<LifecycleListener *> const &lifecycleListeners = clientConfig.getLifecycleListeners();
                listeners.insert(lifecycleListeners.begin(), lifecycleListeners.end());
                fireLifecycleEvent(LifecycleEvent::STARTING);
            };

            void LifecycleService::start() {
                active = true;
                fireLifecycleEvent(LifecycleEvent::STARTED);
                clientContext.getConnectionManager().start();
                clientContext.getInvocationService().start();
                clientContext.getClusterService().start();
                clientContext.getPartitionService().start();
            };


            void LifecycleService::shutdown() {
                active = false;
                boost::lock_guard<boost::mutex> lg(lifecycleLock);
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                clientContext.getConnectionManager().stop();
                clientContext.getClusterService().stop();
                clientContext.getPartitionService().stop();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
            };

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                listeners.insert(lifecycleListener);
            };

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            };

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "STARTING");
                        break;
                    case LifecycleEvent::STARTED :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "STARTED");
                        break;
                    case LifecycleEvent::SHUTTING_DOWN :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "SHUTTING_DOWN");
                        break;
                    case LifecycleEvent::SHUTDOWN :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "SHUTDOWN");
                        break;
                    case LifecycleEvent::CLIENT_CONNECTED :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "CLIENT_CONNECTED");
                        break;
                    case LifecycleEvent::CLIENT_DISCONNECTED :
                        util::ILogger::info("LifecycleService::LifecycleEvent", "CLIENT_DISCONNECTED");
                        break;
                }

                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                };
            };

            bool LifecycleService::isRunning() {
                return active;
            };
        }
    }
}