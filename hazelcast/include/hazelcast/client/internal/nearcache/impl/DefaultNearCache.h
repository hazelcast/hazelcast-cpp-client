/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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

#include <climits>
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    template <typename K, typename V>
                    class DefaultNearCache : public NearCache<K, V> {
                    public:
                        DefaultNearCache(const std::string &cacheName, const boost::shared_ptr<config::NearCacheConfig> &config,
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
                        boost::shared_ptr<V> get(const boost::shared_ptr<K> &key) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on get!");

                            return nearCacheRecordStore->get(key);
                        }

                        //@Override
                        void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<V> &value) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        //@Override
                        void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<serialization::pimpl::Data> &value) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        //@Override
                        bool remove(const boost::shared_ptr<K> &key) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on remove!");

                            return nearCacheRecordStore->remove(key);
                        }

                        //@Override
                        bool isInvalidatedOnChange() const {
                            return nearCacheConfig->isInvalidateOnChange();
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
                            return nearCacheConfig->getInMemoryFormat();
                        }

                        //@Override
                        const boost::shared_ptr<config::NearCachePreloaderConfig> getPreloaderConfig() const {
                            return nearCacheConfig->getPreloaderConfig();
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                         */
/*
                        const NearCacheStats &getNearCacheStats() const;
*/

                        /**
                         * Selects the best candidate object to store from the given <code>candidates</code>.
                         *
                         * @param candidates the candidates from which the best candidate object will be selected.
                         * @return the best candidate object to store, selected from the given <code>candidates</code>.
                         */
/*
                    virtual boost::shared_ptr<V> selectToSave(std::vector<boost::shared_ptr<V> > candidates) const;
*/

                        //@Override
                        int size() const {
                            return nearCacheRecordStore->size();
                        }

/*TODO
                        //@Override
                        void preload(const adapter::DataStructureAdapter<K, V> &adapter) {
                            assert(0);
                        }
*/

                        //@Override
                        void storeKeys() {
                            // we don't store new keys, until the pre-loader is done
                            if (preloadDone) {
                                nearCacheRecordStore->storeKeys();
                            }
                        }

                        bool isPreloadDone() {
                            return preloadDone;
                        }
                    private:
                        std::auto_ptr<NearCacheRecordStore<K, V> > createNearCacheRecordStore(const std::string &name,
                                                                                              const boost::shared_ptr<config::NearCacheConfig> &nearCacheConfig) {
                            config::InMemoryFormat inMemoryFormat = nearCacheConfig->getInMemoryFormat();
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    return std::auto_ptr<NearCacheRecordStore<K, V> >(
                                    new store::NearCacheDataRecordStore<K, V>(name, nearCacheConfig, serializationService));
                                case config::OBJECT:
                                    return std::auto_ptr<NearCacheRecordStore<K, V> >(
                                    new store::NearCacheObjectRecordStore<K, V>(name, nearCacheConfig, serializationService));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << inMemoryFormat;
                                    throw exception::IllegalArgumentException(out.str());
                            }
                        }

                        class ExpirationTask {
                        public:
                            ExpirationTask(NearCacheRecordStore<K, V> &store)
                                    : expirationInProgress(false), nearCacheRecordStore(store),
                                      initialDelayInSeconds(NearCache<K, V>::DEFAULT_EXPIRATION_TASK_INITIAL_DELAY_IN_SECONDS),
                                      periodInSeconds(NearCache<K, V>::DEFAULT_EXPIRATION_TASK_DELAY_IN_SECONDS),
                                      cancelled(false) {
                            }

                            void run() {
                                int periodInMillis = periodInSeconds * 1000;

                                util::sleep(initialDelayInSeconds);

                                while (!cancelled) {
                                    int64_t end = util::currentTimeMillis() + periodInMillis;
                                    if (expirationInProgress.compareAndSet(false, true)) {
                                        try {
                                            nearCacheRecordStore.doExpiration();
                                        } catch (...) {
                                            expirationInProgress = false;
                                            // TODO: What to do here
                                        }
                                    }

                                    // sleep to complete the period
                                    util::sleepmillis(end - util::currentTimeMillis());
                                }
                            }

                            void schedule() {
                                task = std::auto_ptr<util::Thread>(new util::Thread(taskStarter, this));
                            }

                            void cancel() {
                                cancelled = true;
                                task->cancel();
                                task->join();
                            }
                            
                            static void taskStarter(util::ThreadArgs &args) {
                                ExpirationTask *expirationTask = (ExpirationTask *) args.arg0;
                                expirationTask->run();
                            }
                        private:
                            util::AtomicBoolean expirationInProgress;
                            NearCacheRecordStore<K, V> &nearCacheRecordStore;
                            int initialDelayInSeconds;
                            int periodInSeconds;
                            std::auto_ptr<util::Thread> task;
                            util::AtomicBoolean cancelled;
                        };

                        std::auto_ptr<ExpirationTask> createAndScheduleExpirationTask() {
                            if (nearCacheConfig->getMaxIdleSeconds() > 0L || nearCacheConfig->getTimeToLiveSeconds() > 0L) {
                                std::auto_ptr<ExpirationTask> expirationTask(new ExpirationTask(*nearCacheRecordStore));
                                expirationTask->schedule();
                                return expirationTask;
                            }
                            return std::auto_ptr<ExpirationTask>();
                        }

                        const std::string &name;
                        const boost::shared_ptr<config::NearCacheConfig> nearCacheConfig;
                        serialization::pimpl::SerializationService &serializationService;

                        std::auto_ptr<NearCacheRecordStore<K, V> > nearCacheRecordStore;
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

