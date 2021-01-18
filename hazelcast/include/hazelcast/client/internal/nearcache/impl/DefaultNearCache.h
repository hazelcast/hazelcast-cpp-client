/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/config/near_cache_config.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/monitor/near_cache_stats.h"
#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/logger.h"

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
                        DefaultNearCache(const std::string &cache_name,
                                         const client::config::near_cache_config &config,
                                         const std::shared_ptr<spi::impl::ClientExecutionServiceImpl> &es,
                                         serialization::pimpl::SerializationService &ss, logger &lg)
                                : name_(cache_name), near_cache_config_(config), execution_service_(es),
                                  serialization_service_(ss), logger_(lg) {
                        }

                        ~DefaultNearCache() override = default;

                        void initialize() override {
                            if (near_cache_record_store_.get() == NULL) {
                                near_cache_record_store_ = create_near_cache_record_store(name_, near_cache_config_);
                            }
                            near_cache_record_store_->initialize();

                            schedule_expiration_task();
                        }

                        const std::string &get_name() const override {
                            return name_;
                        }

                        std::shared_ptr<V> get(const std::shared_ptr<KS> &key) override {
                            util::Preconditions::check_not_null(key, "key cannot be null on get!");

                            return near_cache_record_store_->get(key);
                        }

                        void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) override {
                            util::Preconditions::check_not_null<KS>(key, "key cannot be null on put!");

                            near_cache_record_store_->do_eviction_if_required();

                            near_cache_record_store_->put(key, value);
                        }

                        //@Override
/*
                        void put(const std::shared_ptr<KS> &key,
                                 const std::shared_ptr<serialization::pimpl::data> &value) {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            nearCacheRecordStore->doEvictionIfRequired();

                            nearCacheRecordStore->put(key, value);
                        }
*/

                        bool invalidate(const std::shared_ptr<KS> &key) override {
                            util::Preconditions::check_not_null<KS>(key, "key cannot be null on invalidate!");

                            return near_cache_record_store_->invalidate(key);
                        }

                        bool is_invalidated_on_change() const override {
                            return near_cache_config_.is_invalidate_on_change();
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

                        const client::config::in_memory_format get_in_memory_format() const override {
                            return near_cache_config_.get_in_memory_format();
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                         */
                        std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats() const override {
                            return near_cache_record_store_->get_near_cache_stats();
                        }

                        int size() const override {
                            return near_cache_record_store_->size();
                        }

                    private:
                        std::unique_ptr<NearCacheRecordStore<KS, V> >
                        create_near_cache_record_store(const std::string &name,
                                                   const client::config::near_cache_config &near_cache_config) {
                            client::config::in_memory_format inMemoryFormat = near_cache_config.get_in_memory_format();
                            switch (inMemoryFormat) {
                                case client::config::BINARY:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheDataRecordStore<K, V, KS>(name, near_cache_config,
                                                                                          serialization_service_));
                                case client::config::OBJECT:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheObjectRecordStore<K, V, KS>(name, near_cache_config,
                                                                                            serialization_service_));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << (int) inMemoryFormat;
                                    BOOST_THROW_EXCEPTION(exception::illegal_argument(out.str()));
                            }
                        }

                        void schedule_expiration_task() {
                            if (near_cache_config_.get_max_idle_seconds() > 0L ||
                                near_cache_config_.get_time_to_live_seconds() > 0L) {
                                expiration_timer_ = execution_service_->schedule_with_repetition(
                                    [=]() {
                                        std::atomic_bool expirationInProgress(false);
                                        while (!expiration_cancelled_) {
                                            bool expected = false;
                                            if (expirationInProgress.compare_exchange_strong(expected, true)) {
                                                try {
                                                    near_cache_record_store_->do_expiration();
                                                } catch (exception::iexception &e) {
                                                    expirationInProgress.store(false);
                                                    // TODO: What to do here
                                                    HZ_LOG(logger_, info,
                                                        boost::str(boost::format(
                                                            "ExpirationTask nearCacheRecordStore.doExpiration failed. "
                                                            "%1% This may NOT be a vital problem since this doExpiration "
                                                            "runs periodically and it should recover eventually.")
                                                            % e.what())
                                                    );
                                                }
                                            }
                                        }
                                    },
                                    std::chrono::seconds(NearCache<K, V>::DEFAULT_EXPIRATION_TASK_INITIAL_DELAY_IN_SECONDS),
                                    std::chrono::seconds(NearCache<K, V>::DEFAULT_EXPIRATION_TASK_DELAY_IN_SECONDS));
                            }
                        }

                        std::string name_;
                        const client::config::near_cache_config &near_cache_config_;
                        std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
                        serialization::pimpl::SerializationService &serialization_service_;
                        logger &logger_;

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



