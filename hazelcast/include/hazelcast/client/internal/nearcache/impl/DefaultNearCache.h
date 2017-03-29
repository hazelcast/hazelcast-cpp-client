/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHE_H_

#include <string>
#include <memory>

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    template<typename K, typename V, typename KS>
                    class DefaultNearCache : public NearCache<KS, V> {
                    public:
                        DefaultNearCache(const std::string &cacheName, const config::NearCacheConfig<K, V> &config,
                                         serialization::pimpl::SerializationService &ss)
                                : name(cacheName), nearCacheConfig(config), serializationService(ss) {
                        }

                        virtual ~DefaultNearCache() {
                        }

                        //@Override
                        void initialize() {
                            if (nearCacheRecordStore.get() == NULL) {
                                nearCacheRecordStore = createNearCacheRecordStore(name, nearCacheConfig);
                            }
                            nearCacheRecordStore->initialize();

                            expirationTaskFuture = createAndScheduleExpirationTask();
                        }

                        //@Override
                        const std::string &getName() const {
                            return name;
                        }

                        //@Override
                        boost::shared_ptr<V> get(const boost::shared_ptr<KS> &key) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on get!");

                            return nearCacheRecordStore->get(key);
                        }

                        //@Override
                        void put(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        //@Override
                        void put(const boost::shared_ptr<KS> &key,
                                 const boost::shared_ptr<serialization::pimpl::Data> &value) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        //@Override
                        bool remove(const boost::shared_ptr<KS> &key) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on remove!");

                            return nearCacheRecordStore->remove(key);
                        }

                        //@Override
                        bool isInvalidatedOnChange() const {
                            return nearCacheConfig.isInvalidateOnChange();
                        }

                        //@Override
                        void clear() {
                            nearCacheRecordStore->clear();
                        }

                        //@Override
                        void destroy() {
                            if (NULL != expirationTaskFuture.get()) {
                                expirationTaskFuture->cancel();
                            }
                            nearCacheRecordStore->destroy();
                        }

                        //@Override
                        const config::InMemoryFormat getInMemoryFormat() const {
                            return nearCacheConfig.getInMemoryFormat();
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                         */
                        monitor::NearCacheStats &getNearCacheStats() {
                            return nearCacheRecordStore->getNearCacheStats();
                        }

                        //@Override
                        int size() const {
                            return nearCacheRecordStore->size();
                        }

                    private:
                        std::auto_ptr<NearCacheRecordStore<KS, V> > createNearCacheRecordStore(const std::string &name,
                                                                                               const config::NearCacheConfig<K, V> &nearCacheConfig) {
                            config::InMemoryFormat inMemoryFormat = nearCacheConfig.getInMemoryFormat();
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    return std::auto_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheDataRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                          serializationService));
                                case config::OBJECT:
                                    return std::auto_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheObjectRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                            serializationService));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << (int) inMemoryFormat;
                                    throw exception::IllegalArgumentException(out.str());
                            }
                        }

                        class ExpirationTask {
                        public:
                            ExpirationTask(const std::string &mapName, NearCacheRecordStore<KS, V> &store)
                                    : expirationInProgress(false), nearCacheRecordStore(store),
                                      initialDelayInSeconds(
                                              NearCache<K, V>::DEFAULT_EXPIRATION_TASK_INITIAL_DELAY_IN_SECONDS),
                                      periodInSeconds(NearCache<K, V>::DEFAULT_EXPIRATION_TASK_DELAY_IN_SECONDS),
                                      cancelled(false), name(mapName) {
                            }

                            void run() {
                                std::ostringstream out;
                                out << "Near cache expiration thread started for map " << name << ". The period is:" <<
                                periodInSeconds << " seconds.";
                                util::ILogger::getLogger().info(out.str());
                                int periodInMillis = periodInSeconds * 1000;

                                util::sleep(initialDelayInSeconds);

                                while (!cancelled) {
                                    int64_t end = util::currentTimeMillis() + periodInMillis;
                                    if (expirationInProgress.compareAndSet(false, true)) {
                                        try {
                                            nearCacheRecordStore.doExpiration();
                                        } catch (exception::IException &e) {
                                            expirationInProgress = false;
                                            // TODO: What to do here
                                            std::ostringstream out;
                                            out << "ExpirationTask nearCacheRecordStore.doExpiration failed. "
                                            << e.what() << " This mat NOT a vital problem since this doExpiration runs "
                                                    "periodically every " << periodInMillis <<
                                            "milliseconds and it should recover eventually.";
                                            util::ILogger::getLogger().info(out.str());
                                        }
                                    }

                                    // sleep to complete the period
                                    int64_t now = util::currentTimeMillis();
                                    if (end > now) {
                                        util::sleepmillis((uint64_t) end - now);
                                    }
                                }
                            }

                            void schedule() {
                                task = std::auto_ptr<util::Thread>(new util::Thread(taskStarter, this));
                            }

                            void cancel() {
                                cancelled = true;
                                task->cancel();
                                task->join();
                                std::ostringstream out;
                                out << "Near cache expiration thread is stopped for map "
                                << name << " since near cache is being destroyed.";
                                util::ILogger::getLogger().info(out.str());
                            }

                            static void taskStarter(util::ThreadArgs &args) {
                                ExpirationTask *expirationTask = (ExpirationTask *) args.arg0;
                                expirationTask->run();
                            }
                        private:
                            util::AtomicBoolean expirationInProgress;
                            NearCacheRecordStore<KS, V> &nearCacheRecordStore;
                            int initialDelayInSeconds;
                            int periodInSeconds;
                            std::auto_ptr<util::Thread> task;
                            util::AtomicBoolean cancelled;
                            std::string name;
                        };

                        std::auto_ptr<ExpirationTask> createAndScheduleExpirationTask() {
                            if (nearCacheConfig.getMaxIdleSeconds() > 0L ||
                                nearCacheConfig.getTimeToLiveSeconds() > 0L) {
                                std::auto_ptr<ExpirationTask> expirationTask(
                                        new ExpirationTask(name, *nearCacheRecordStore));
                                expirationTask->schedule();
                                return expirationTask;
                            }
                            return std::auto_ptr<ExpirationTask>();
                        }

                        const std::string &name;
                        const config::NearCacheConfig<K, V> &nearCacheConfig;
                        serialization::pimpl::SerializationService &serializationService;

                        std::auto_ptr<NearCacheRecordStore<KS, V> > nearCacheRecordStore;
                        std::auto_ptr<ExpirationTask> expirationTaskFuture;

                        util::AtomicBoolean preloadDone;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHE_H_ */

