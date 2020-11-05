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

#include <memory>

#include "hazelcast/client/IMap.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            /**
             * A Client-side {@code IMap} implementation which is fronted by a near-cache.
             *
             * @param <K> the key type for this {@code IMap} proxy.
             * @param <V> the value type for this {@code IMap} proxy.
             */
            template<typename K = serialization::pimpl::Data, typename V = serialization::pimpl::Data>
            class NearCachedClientMapProxy
                    : public IMap,
                      public std::enable_shared_from_this<NearCachedClientMapProxy<K, V> > {
            public:
                NearCachedClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                        : IMap(instanceName, context), cacheLocalEntries_(false),
                          invalidateOnChange_(false), keyStateMarker_(NULL),
                          nearCacheConfig_(*context->get_client_config().get_near_cache_config(instanceName)),
                          logger_(context->get_logger()) {}

            protected:
                typedef std::unordered_map<std::shared_ptr<serialization::pimpl::Data>, bool> MARKER_MAP;

                void on_initialize() override {
                    IMap::on_initialize();

                    internal::nearcache::NearCacheManager &nearCacheManager = this->get_context().get_near_cache_manager();
                    cacheLocalEntries_ = nearCacheConfig_.is_cache_local_entries();
                    int partitionCount = this->get_context().get_partition_service().get_partition_count();
                    nearCache_ = nearCacheManager.get_or_create_near_cache<K, V, serialization::pimpl::Data>(
                            spi::ClientProxy::get_name(), nearCacheConfig_);

                    nearCache_ = impl::nearcache::InvalidationAwareWrapper<
                            serialization::pimpl::Data, V>::as_invalidation_aware(nearCache_, partitionCount);

                    keyStateMarker_ = get_key_state_marker();

                    invalidateOnChange_ = nearCache_->is_invalidated_on_change();
                    if (invalidateOnChange_) {
                        std::shared_ptr<client::impl::BaseEventHandler> invalidationHandler(
                                new ClientMapAddNearCacheEventHandler(nearCache_));
                        add_near_cache_invalidate_listener(invalidationHandler);
                    }

                    localMapStats_ = monitor::impl::LocalMapStatsImpl(nearCache_->get_near_cache_stats());
                }

                void post_destroy() override {
                    try {
                        remove_near_cache_invalidation_listener();
                        spi::ClientProxy::get_context().get_near_cache_manager().destroy_near_cache(
                                spi::ClientProxy::get_name());
                        spi::ClientProxy::post_destroy();
                    } catch (exception::IException &) {
                        spi::ClientProxy::post_destroy();
                    }
                }

                void on_destroy() override {
                    remove_near_cache_invalidation_listener();
                    spi::ClientProxy::get_context().get_near_cache_manager().destroy_near_cache(spi::ClientProxy::get_name());

                    IMap::on_shutdown();
                }

                boost::future<bool> contains_key_internal(const serialization::pimpl::Data &keyData) override {
                    auto key = std::make_shared<serialization::pimpl::Data>(keyData);
                    std::shared_ptr<V> cached = nearCache_->get(key);
                    if (cached) {
                        return boost::make_ready_future(internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached);
                    }

                    return IMap::contains_key_internal(*key);
                }

                boost::future<boost::optional<serialization::pimpl::Data>> get_internal(const serialization::pimpl::Data &keyData) override {
                    auto key = std::make_shared<serialization::pimpl::Data>(keyData);
                    auto cached = nearCache_->get(key);
                    if (cached) {
                        if (internal::nearcache::NearCache<K, V>::NULL_OBJECT == cached) {
                            return boost::make_ready_future(boost::optional<serialization::pimpl::Data>());
                        }
                        return boost::make_ready_future(boost::make_optional(*cached));
                    }

                    bool marked = keyStateMarker_->try_mark(*key);

                    try {
                        auto future = IMap::get_internal(*key);
                        if (marked) {
                            return future.then(boost::launch::deferred, [=](boost::future<boost::optional<serialization::pimpl::Data>> f) {
                                auto data = f.get();
                                auto cachedValue = data ? std::make_shared<serialization::pimpl::Data>(*data)
                                                        : internal::nearcache::NearCache<K, V>::NULL_OBJECT;
                                try_to_put_near_cache(key, cachedValue);
                                return data;
                            });
                        }
                        return future;
                    } catch (exception::IException &) {
                        reset_to_unmarked_state(key);
                        throw;
                    }
                }

                boost::future<bool> remove_internal(
                        const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) override {
                    try {
                        auto response = IMap::remove_internal(key, value);
                        invalidate_near_cache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(key);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>> remove_internal(
                        const serialization::pimpl::Data &keyData) override {
                    try {
                        invalidate_near_cache(keyData);
                        auto response = IMap::remove_internal(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                remove_all_internal(const serialization::pimpl::Data &predicateData) override {
                    try {
                        auto response = IMap::remove_all_internal(predicateData);
                        nearCache_->clear();
                        return response;
                    } catch (exception::IException &) {
                        nearCache_->clear();
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage> delete_internal(const serialization::pimpl::Data &key) override {
                    try {
                        auto response = IMap::delete_internal(key);
                        invalidate_near_cache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(key);
                        throw;
                    }
                }

                boost::future<bool> try_remove_internal(const serialization::pimpl::Data &keyData,
                                                              std::chrono::milliseconds timeout) override {
                    try {
                        auto response = IMap::try_remove_internal(keyData, timeout);
                        invalidate_near_cache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<bool> try_put_internal(const serialization::pimpl::Data &keyData,
                        const serialization::pimpl::Data &valueData, std::chrono::milliseconds timeout) override {
                    try {
                        auto response = IMap::try_put_internal(keyData, valueData, timeout);
                        invalidate_near_cache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>> put_internal(const serialization::pimpl::Data &keyData,
                        const serialization::pimpl::Data &valueData, std::chrono::milliseconds ttl) override {
                    try {
                        auto previousValue = IMap::put_internal(keyData, valueData, ttl);
                        invalidate_near_cache(keyData);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                try_put_transient_internal(const serialization::pimpl::Data &keyData,
                                        const serialization::pimpl::Data &valueData,
                                        std::chrono::milliseconds ttl) override {
                    try {
                        auto result = IMap::try_put_transient_internal(keyData, valueData, ttl);
                        invalidate_near_cache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>>
                put_if_absent_internal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    std::chrono::milliseconds ttl) override {
                    try {
                        auto previousValue = IMap::put_if_absent_data(keyData, valueData, ttl);
                        invalidate_near_cache(keyData);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<bool> replace_if_same_internal(const serialization::pimpl::Data &keyData,
                                                          const serialization::pimpl::Data &valueData,
                                                          const serialization::pimpl::Data &newValueData) override {
                    try {
                        auto result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                        invalidate_near_cache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>>
                replace_internal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) override {
                    try {
                        auto value = proxy::IMapImpl::replace_data(keyData, valueData);
                        invalidate_near_cache(keyData);
                        return value;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                set_internal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            std::chrono::milliseconds ttl) override {
                    try {
                        auto result = proxy::IMapImpl::set(keyData, valueData, ttl);
                        invalidate_near_cache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<bool> evict_internal(const serialization::pimpl::Data &keyData) override {
                    try {
                        auto evicted = proxy::IMapImpl::evict(keyData);
                        invalidate_near_cache(keyData);
                        return evicted;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<EntryVector>
                get_all_internal(int partitionId, const std::vector<serialization::pimpl::Data> &partitionKeys) override {
                    auto markers = std::make_shared<MARKER_MAP>();
                    try {
                        EntryVector result;
                        std::vector<serialization::pimpl::Data> remainingKeys;
                        for (auto &key : partitionKeys) {
                            auto keyData = std::make_shared<serialization::pimpl::Data>(key);
                            auto cached = nearCache_->get(keyData);
                            if (cached && internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached) {
                                result.push_back(std::make_pair(std::move(*keyData), *cached));
                            } else {
                                if (invalidateOnChange_) {
                                    (*markers)[keyData] = keyStateMarker_->try_mark(*keyData);
                                }
                                remainingKeys.push_back(std::move(*keyData));
                            }
                        }

                        if (remainingKeys.empty()) {
                            return boost::make_ready_future(result);
                        }

                        return IMap::get_all_internal(partitionId, remainingKeys).then(
                                boost::launch::deferred, [=](boost::future<EntryVector> f) {
                            EntryVector allEntries(result);
                            for (auto &entry : f.get()) {
                                auto key = std::make_shared<serialization::pimpl::Data>(std::move(entry.first));
                                auto value = std::make_shared<serialization::pimpl::Data>(std::move(entry.second));
                                bool marked = false;
                                auto foundEntry = markers->find(key);
                                if (foundEntry != markers->end()) {
                                    marked = foundEntry->second;
                                    markers->erase(foundEntry);
                                }

                                if (marked) {
                                    try_to_put_near_cache(key, value);
                                } else {
                                    nearCache_->put(key, value);
                                }
                                allEntries.push_back(std::make_pair(std::move(*key), std::move(*value)));
                            }

                            unmark_remaining_marked_keys(*markers);
                            return allEntries;
                        });
                    } catch (exception::IException &) {
                        unmark_remaining_marked_keys(*markers);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>>
                execute_on_key_internal(const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &processor) override {
                    try {
                        auto response = IMap::execute_on_key_data(keyData, processor);
                        invalidate_near_cache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidate_near_cache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                put_all_internal(int partitionId, const EntryVector &entries) override {
                    try {
                        auto result = IMap::put_all_internal(partitionId, entries);
                        invalidate_entries(entries);
                        return result;
                    } catch (exception::IException &) {
                        invalidate_entries(entries);
                        throw;
                    }
                }

                void invalidate_entries(const EntryVector &entries) {
                    for (auto &entry : entries) {
                        invalidate_near_cache(std::make_shared<serialization::pimpl::Data>(entry.first));
                    }
                }

            private:
                impl::nearcache::KeyStateMarker *get_key_state_marker() {
                    return std::static_pointer_cast<
                            impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, V> >(nearCache_)->
                            get_key_state_marker();
                }

                void add_near_cache_invalidate_listener(std::shared_ptr<client::impl::BaseEventHandler> &handler) {
                    try {
                        invalidationListenerId_ = proxy::ProxyImpl::register_listener(create_near_cache_entry_listener_codec(),
                                                                                    handler).get();
                    } catch (exception::IException &e) {
                        HZ_LOG(logger_, severe, 
                            boost::str(boost::format("Near Cache is not initialized!!! %1%") % e.what()) 
                        );
                    }
                }

                void remove_near_cache_invalidation_listener() {
                    if (invalidationListenerId_.is_nil()) {
                        return;
                    }

                    proxy::ProxyImpl::deregister_listener(invalidationListenerId_).get();
                }

                class ClientMapAddNearCacheEventHandler
                        : public protocol::codec::map_addnearcacheinvalidationlistener_handler {
                public:
                    // TODO: implement RepairingTask as in Java client
                    ClientMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > &cache)
                            : nearCache_(cache) {
                    }

                    void before_listener_register() override {
                        nearCache_->clear();
                    }

                    void on_listener_register() override {
                        nearCache_->clear();
                    }

                    void handle_imapinvalidation(const boost::optional<Data> &key, boost::uuids::uuid sourceUuid,
                                                 boost::uuids::uuid partitionUuid, int64_t sequence) override {
                        // null key means Near Cache has to remove all entries in it (see MapAddNearCacheEntryListenerMessageTask)
                        if (!key) {
                            nearCache_->clear();
                        } else {
                            nearCache_->invalidate(std::make_shared<serialization::pimpl::Data>(*key));
                        }
                    }

                    void handle_imapbatchinvalidation(const std::vector<serialization::pimpl::Data> &keys,
                                                      const std::vector<boost::uuids::uuid> &sourceUuids,
                                                      const std::vector<boost::uuids::uuid> &partitionUuids,
                                                      const std::vector<int64_t> &sequences) override {
                        for (auto &k : keys) {
                            nearCache_->invalidate(std::make_shared<serialization::pimpl::Data>(k));
                        }
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache_;
                };

                class NearCacheEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    protocol::ClientMessage encode_add_request(bool localOnly) const override {
                        return protocol::codec::map_addnearcacheinvalidationlistener_encode(name_,
                                                                                                 static_cast<int32_t>(listenerFlags_),
                                                                                                 localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::map_removeentrylistener_encode(name_, realRegistrationId);
                    }

                    NearCacheEntryListenerMessageCodec(const std::string &name, EntryEvent::type listenerFlags)
                            : name_(name), listenerFlags_(listenerFlags) {}

                private:
                    const std::string &name_;
                    EntryEvent::type listenerFlags_;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_near_cache_entry_listener_codec() {
                    EntryEvent::type listenerFlags = EntryEvent::type::INVALIDATION;
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheEntryListenerMessageCodec(spi::ClientProxy::get_name(), listenerFlags));
                }

                void reset_to_unmarked_state(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (keyStateMarker_->try_unmark(*key)) {
                        return;
                    }

                    invalidate_near_cache(key);
                    keyStateMarker_->force_unmark(*key);
                }

                void unmark_remaining_marked_keys(MARKER_MAP &markers) {
                    for (MARKER_MAP::const_iterator it = markers.begin();
                         it != markers.end(); ++it) {
                        if (it->second) {
                            keyStateMarker_->force_unmark(*it->first);
                        }
                    }
                }

                void try_to_put_near_cache(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                       const std::shared_ptr<V> &response) {
                    try_to_put_near_cache_internal<V>(keyData, response);
                }

/*
                void tryToPutNearCache(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                       const std::shared_ptr<serialization::pimpl::Data> &response) {
                    tryToPutNearCacheInternal<serialization::pimpl::Data>(keyData, response);
                }
*/

                template<typename VALUETYPE>
                void try_to_put_near_cache_internal(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                               const std::shared_ptr<VALUETYPE> &response) {
                    try {
                        nearCache_->put(keyData, response);
                        reset_to_unmarked_state(keyData);
                    } catch (exception::IException &) {
                        reset_to_unmarked_state(keyData);
                        throw;
                    }
                }

                /**
                 * This method modifies the key Data internal pointer although it is marked as const
                 * @param key The key for which to invalidate the near cache
                 */
                void invalidate_near_cache(const serialization::pimpl::Data &key) {
                    nearCache_->invalidate(std::make_shared<serialization::pimpl::Data>(key));
                }

                void invalidate_near_cache(std::shared_ptr<serialization::pimpl::Data> key) {
                    nearCache_->invalidate(key);
                }

                bool cacheLocalEntries_;
                bool invalidateOnChange_;
                impl::nearcache::KeyStateMarker *keyStateMarker_;
                const config::NearCacheConfig &nearCacheConfig_;
                std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V>> nearCache_;
                boost::uuids::uuid invalidationListenerId_;
                logger &logger_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

