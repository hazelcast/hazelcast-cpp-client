/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 6/17/13.

#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/impl/statistics/Statistics.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            LifecycleService::LifecycleService(ClientContext &clientContext,
                                               const std::set<LifecycleListener *> &lifecycleListeners,
                                               LoadBalancer *const loadBalancer, Cluster &cluster) : clientContext(
                    clientContext), loadBalancer(loadBalancer), cluster(cluster) {
                listeners.insert(lifecycleListeners.begin(), lifecycleListeners.end());
            }

            bool LifecycleService::start() {
                bool expected = false;
                if (!active.compare_exchange_strong(expected, true)) {
                    return false;
                }

                fireLifecycleEvent(LifecycleEvent::STARTED);

                clientContext.getClientExecutionService().start();

                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).start();

                if (!((impl::AbstractClientInvocationService &) clientContext.getInvocationService()).start()) {
                    return false;
                }

                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).start();

                if (!clientContext.getConnectionManager().start()) {
                    return false;
                }

                loadBalancer->init(cluster);

                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).start();

                clientContext.getClientstatistics().start();

                return true;
            }

            void LifecycleService::shutdown() {
                bool expected = true;
                if (!active.compare_exchange_strong(expected, false)) {
                    return;
                }
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                clientContext.getClientExecutionService().shutdown();
                clientContext.getProxyManager().destroy();
                clientContext.getConnectionManager().shutdown();
                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).shutdown();
                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).stop();
                ((spi::impl::AbstractClientInvocationService &) clientContext.getInvocationService()).shutdown();
                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).shutdown();
                clientContext.getNearCacheManager().destroyAllNearCaches();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
                clientContext.getSerializationService().dispose();
            }

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                listeners.insert(lifecycleListener);
            }

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            }

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                util::LockGuard lg(listenerLock);
                util::ILogger &logger = clientContext.getLogger();
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::gitDateToHazelcastLogDate(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        logger.info(msg);
                        break;
                    }
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
                    (*it)->stateChanged(lifecycleEvent);
                }

            }

            bool LifecycleService::isRunning() {
                return active;
            }

            LifecycleService::~LifecycleService() {
                if (active) {
                    shutdown();
                }
            }
        }
    }
}
