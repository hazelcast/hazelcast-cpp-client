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

            bool LifecycleService::start() {
                active = true;
                fireLifecycleEvent(LifecycleEvent::STARTED);
                if (!clientContext.getConnectionManager().start()) {
                    return false;
                }
                if (!clientContext.getClusterService().start()) {
                    return false;
                }
                clientContext.getInvocationService().start();
                if (!clientContext.getPartitionService().start()) {
                    return false;
                }
                return true;
            }

            void LifecycleService::shutdown() {
                util::LockGuard lg(lifecycleLock);
                if(!active)
                    return;
                active = false;
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
		util::ILogger::getLogger().severe("ConnMan stop");
                clientContext.getConnectionManager().stop();
		util::ILogger::getLogger().severe("ClusterService stop");
                clientContext.getClusterService().stop();
		util::ILogger::getLogger().severe("PartitionService Stop");
                clientContext.getPartitionService().stop();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
            };

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                listeners.insert(lifecycleListener);
            };

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            };

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                util::LockGuard lg(listenerLock);
                util::ILogger &logger = util::ILogger::getLogger();
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING :
                        logger.info("LifecycleService::LifecycleEvent STARTING");
                        break;
                    case LifecycleEvent::STARTED :
                        logger.info("LifecycleService::LifecycleEvent STARTED");
                        break;
                    case LifecycleEvent::SHUTTING_DOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTTING_DOWN");
                        break;
                    case LifecycleEvent::SHUTDOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTDOWN");
                        break;
                    case LifecycleEvent::CLIENT_CONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_CONNECTED");
                        break;
                    case LifecycleEvent::CLIENT_DISCONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");
                        break;
                }

		for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    util::ILogger::getLogger().severe("calling registered listeners");
		    (*it)->stateChanged(lifecycleEvent);
		    util::ILogger::getLogger().severe("end calling registered listeners");	                
		}
		
            };

            bool LifecycleService::isRunning() {
                return active;
            };
        }
    }
}
