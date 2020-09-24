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
#pragma once

#include <string>
#include <memory>
#include <thread>

#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/monitor/NearCacheStats.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/ILogger.h"

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
                                : name_(cacheName), near_cache_config_(config), execution_service_(es),
                                  serialization_service_(ss), logger_(logger) {
                        }

                        ~DefaultNearCache() override = default;

                        void initialize() override {
                            if (near_cache_record_store_.get() == NULL) {
                                near_cache_record_store_ = createNearCacheRecordStore(name_, near_cache_config_);
                            }
                            near_cache_record_store_->initialize();

                            scheduleExpirationTask();
                        }

                        const std::string &getName() const override {
                            return name_;
                        }

                        std::shared_ptr<V> get(const std::shared_ptr<KS> &key) override {
                            util::Preconditions::checkNotNull(key, "key cannot be null on get!");

                            return near_cache_record_store_->get(key);
                        }

                        void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) override {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            near_cache_record_store_->doEvictionIfRequired();

                            near_cache_record_store_->put(key, value);
                        }

                        //@Override
/*
                        void put(const std::shared_ptr<KS> &key,
                                 const std::shared_ptr<serialization::pimpl::Data> &value) {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }
*/

                        bool invalidate(const std::shared_ptr<KS> &key) override {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on invalidate!");

                            return near_cache_record_store_->invalidate(key);
                        }

                        bool isInvalidatedOnChange() const override {
                            return near_cache_config_.isInvalidateOnChange();
                        }

                        void clear() override {
                            near_cache_record_store_->clear();
                        }

                        void destroy() override {
                            expiration_cancelled_.store(true);
                            if (expiration_timer_) {
                                boost::system::error_code ignored;
                                expiration_timer_->cancel(ignored);
                            }
                            near_cache_record_store_->destroy();
                        }

                        const client::config::InMemoryFormat getInMemoryFormat() const override {
                            return near_cache_config_.getInMemoryFormat();
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                         */
                        std::shared_ptr<monitor::NearCacheStats> getNearCacheStats() const override {
                            return near_cache_record_store_->getNearCacheStats();
                        }

                        int size() const override {
                            return near_cache_record_store_->size();
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
                                                                                          serialization_service_));
                                case client::config::OBJECT:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheObjectRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                            serialization_service_));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << (int) inMemoryFormat;
                                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException(out.str()));
                            }
                        }

                        void scheduleExpirationTask() {
                            if (near_cache_config_.getMaxIdleSeconds() > 0L ||
                                near_cache_config_.getTimeToLiveSeconds() > 0L) {
                                expiration_timer_ = execution_service_->scheduleWithRepetition([=]() {
                                                                                               std::atomic_bool expirationInProgress(false);
                                                                                               while (!expiration_cancelled_) {
                                                                                                   bool expected = false;
                                                                                                   if (expirationInProgress.compare_exchange_strong(expected, true)) {
                                                                                                       try {
                                                                                                           near_cache_record_store_->doExpiration();
                                                                                                       } catch (exception::IException &e) {
                                                                                                           expirationInProgress.store(false);
                                                                                                           // TODO: What to do here
                                                                                                           logger_.info("ExpirationTask nearCacheRecordStore.doExpiration failed. ",
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

                        const std::string &name_;
                        const client::config::NearCacheConfig<K, V> &near_cache_config_;
                        std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
                        serialization::pimpl::SerializationService &serialization_service_;
                        util::ILogger &logger_;

                        std::unique_ptr<NearCacheRecordStore<KS, V> > near_cache_record_store_;
                        std::atomic_bool expiration_cancelled_;
                        std::shared_ptr<boost::asio::steady_timer> expiration_timer_;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



