/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <thread>

#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"

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
                        DefaultNearCache(const std::string &cacheName,
                                         const client::config::NearCacheConfig<K, V> &config,
                                         const std::shared_ptr<spi::impl::ClientExecutionServiceImpl> &es,
                                         serialization::pimpl::SerializationService &ss, util::ILogger &logger)
                                : name(cacheName), nearCacheConfig(config), executionService(es),
                                  serializationService(ss), logger(logger) {
                        }

                        virtual ~DefaultNearCache() {
                        }

                        //@Override
                        void initialize() {
                            if (nearCacheRecordStore.get() == NULL) {
                                nearCacheRecordStore = createNearCacheRecordStore(name, nearCacheConfig);
                            }
                            nearCacheRecordStore->initialize();

                            scheduleExpirationTask();
                        }

                        //@Override
                        const std::string &getName() const {
                            return name;
                        }

                        //@Override
                        std::shared_ptr<V> get(const std::shared_ptr<KS> &key) {
                            util::Preconditions::checkNotNull(key, "key cannot be null on get!");

                            return nearCacheRecordStore->get(key);
                        }

                        //@Override
                        void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        //@Override
                        void put(const std::shared_ptr<KS> &key,
                                 const std::shared_ptr<serialization::pimpl::Data> &value) {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }

                        bool invalidate(const std::shared_ptr<KS> &key) {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on invalidate!");

                            return nearCacheRecordStore->invalidate(key);
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
                            expiration_cancelled_.store(true);
                            nearCacheRecordStore->destroy();
                        }

                        //@Override
                        const client::config::InMemoryFormat getInMemoryFormat() const {
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
                        std::unique_ptr<NearCacheRecordStore<KS, V> >
                        createNearCacheRecordStore(const std::string &name,
                                                   const client::config::NearCacheConfig<K, V> &nearCacheConfig) {
                            client::config::InMemoryFormat inMemoryFormat = nearCacheConfig.getInMemoryFormat();
                            switch (inMemoryFormat) {
                                case client::config::BINARY:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheDataRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                          serializationService));
                                case client::config::OBJECT:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheObjectRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                            serializationService));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << (int) inMemoryFormat;
                                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException(out.str()));
                            }
                        }

                        void scheduleExpirationTask() {
                            if (nearCacheConfig.getMaxIdleSeconds() > 0L ||
                                nearCacheConfig.getTimeToLiveSeconds() > 0L) {
                                executionService->scheduleWithRepetition([=]() {
                                                                             std::atomic_bool expirationInProgress(false);
                                                                             while (!expiration_cancelled_) {
                                                                                 bool expected = false;
                                                                                 if (expirationInProgress.compare_exchange_strong(expected, true)) {
                                                                                     try {
                                                                                         nearCacheRecordStore->doExpiration();
                                                                                     } catch (exception::IException &e) {
                                                                                         expirationInProgress.store(false);
                                                                                         // TODO: What to do here
                                                                                         logger.info("ExpirationTask nearCacheRecordStore.doExpiration failed. ",
                                                                                                     e.what(),
                                                                                                     " This may NOT be a vital problem since this doExpiration "
                                                                                                     "runs periodically and it should recover eventually.");
                                                                                     }
                                                                                 }
                                                                             }
                                                                         }, std::chrono::seconds(
                                        NearCache<K, V>::DEFAULT_EXPIRATION_TASK_INITIAL_DELAY_IN_SECONDS),
                                                                         std::chrono::seconds(
                                                                                 NearCache<K, V>::DEFAULT_EXPIRATION_TASK_DELAY_IN_SECONDS));
                            }
                        }

                        const std::string &name;
                        const client::config::NearCacheConfig<K, V> &nearCacheConfig;
                        std::shared_ptr<spi::impl::ClientExecutionServiceImpl> executionService;
                        serialization::pimpl::SerializationService &serializationService;
                        util::ILogger &logger;

                        std::unique_ptr<NearCacheRecordStore<KS, V> > nearCacheRecordStore;
                        std::atomic_bool expiration_cancelled_;
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

