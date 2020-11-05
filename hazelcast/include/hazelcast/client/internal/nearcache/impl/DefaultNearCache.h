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
                        DefaultNearCache(const std::string &cacheName,
                                         const client::config::NearCacheConfig &config,
                                         const std::shared_ptr<spi::impl::ClientExecutionServiceImpl> &es,
                                         serialization::pimpl::SerializationService &ss, logger &lg)
                                : name_(cacheName), nearCacheConfig_(config), executionService_(es),
                                  serializationService_(ss), logger_(lg) {
                        }

                        ~DefaultNearCache() override = default;

                        void initialize() override {
                            if (nearCacheRecordStore_.get() == NULL) {
                                nearCacheRecordStore_ = createNearCacheRecordStore(name_, nearCacheConfig_);
                            }
                            nearCacheRecordStore_->initialize();

                            scheduleExpirationTask();
                        }

                        const std::string &getName() const override {
                            return name_;
                        }

                        std::shared_ptr<V> get(const std::shared_ptr<KS> &key) override {
                            util::Preconditions::checkNotNull(key, "key cannot be null on get!");

                            return nearCacheRecordStore_->get(key);
                        }

                        void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) override {
                            util::Preconditions::checkNotNull<KS>(key, "key cannot be null on put!");

                            nearCacheRecordStore_->doEvictionIfRequired();

                            nearCacheRecordStore_->put(key, value);
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

                            return nearCacheRecordStore_->invalidate(key);
                        }

                        bool isInvalidatedOnChange() const override {
                            return nearCacheConfig_.isInvalidateOnChange();
                        }

                        void clear() override {
                            nearCacheRecordStore_->clear();
                        }

                        void destroy() override {
                            expiration_cancelled_.store(true);
                            if (expirationTimer_) {
                                boost::system::error_code ignored;
                                expirationTimer_->cancel(ignored);
                            }
                            nearCacheRecordStore_->destroy();
                        }

                        const client::config::InMemoryFormat getInMemoryFormat() const override {
                            return nearCacheConfig_.getInMemoryFormat();
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                         */
                        std::shared_ptr<monitor::NearCacheStats> getNearCacheStats() const override {
                            return nearCacheRecordStore_->getNearCacheStats();
                        }

                        int size() const override {
                            return nearCacheRecordStore_->size();
                        }

                    private:
                        std::unique_ptr<NearCacheRecordStore<KS, V> >
                        createNearCacheRecordStore(const std::string &name,
                                                   const client::config::NearCacheConfig &nearCacheConfig) {
                            client::config::InMemoryFormat inMemoryFormat = nearCacheConfig.getInMemoryFormat();
                            switch (inMemoryFormat) {
                                case client::config::BINARY:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheDataRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                          serializationService_));
                                case client::config::OBJECT:
                                    return std::unique_ptr<NearCacheRecordStore<KS, V> >(
                                            new store::NearCacheObjectRecordStore<K, V, KS>(name, nearCacheConfig,
                                                                                            serializationService_));
                                default:
                                    std::ostringstream out;
                                    out << "Invalid in memory format: " << (int) inMemoryFormat;
                                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException(out.str()));
                            }
                        }

                        void scheduleExpirationTask() {
                            if (nearCacheConfig_.getMaxIdleSeconds() > 0L ||
                                nearCacheConfig_.getTimeToLiveSeconds() > 0L) {
                                expirationTimer_ = executionService_->scheduleWithRepetition(
                                    [=]() {
                                        std::atomic_bool expirationInProgress(false);
                                        while (!expiration_cancelled_) {
                                            bool expected = false;
                                            if (expirationInProgress.compare_exchange_strong(expected, true)) {
                                                try {
                                                    nearCacheRecordStore_->doExpiration();
                                                } catch (exception::IException &e) {
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

                        const std::string &name_;
                        const client::config::NearCacheConfig &nearCacheConfig_;
                        std::shared_ptr<spi::impl::ClientExecutionServiceImpl> executionService_;
                        serialization::pimpl::SerializationService &serializationService_;
                        logger &logger_;

                        std::unique_ptr<NearCacheRecordStore<KS, V> > nearCacheRecordStore_;
                        std::atomic_bool expiration_cancelled_;
                        std::shared_ptr<boost::asio::steady_timer> expirationTimer_;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



