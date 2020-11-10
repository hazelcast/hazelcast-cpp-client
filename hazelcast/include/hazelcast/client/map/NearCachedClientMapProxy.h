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

#include "hazelcast/client/imap.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/config/near_cache_config.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/entry_event.h"
#include "hazelcast/client/execution_callback.h"
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
            template<typename K = serialization::pimpl::data, typename V = serialization::pimpl::data>
            class NearCachedClientMapProxy
                    : public imap,
                      public std::enable_shared_from_this<NearCachedClientMapProxy<K, V> > {
            public:
                NearCachedClientMapProxy(const std::string &instance_name, spi::ClientContext *context)
                        : imap(instance_name, context), cache_local_entries_(false),
                          invalidate_on_change_(false), key_state_marker_(NULL),
                          near_cache_config_(*context->get_client_config().get_near_cache_config(instance_name)),
                          logger_(context->get_logger()) {}

            protected:
                typedef std::unordered_map<std::shared_ptr<serialization::pimpl::data>, bool> MARKER_MAP;

                void on_initialize() override {
                    imap::on_initialize();

                    internal::nearcache::NearCacheManager &nearCacheManager = this->get_context().get_near_cache_manager();
                    cache_local_entries_ = near_cache_config_.is_cache_local_entries();
                    int partitionCount = this->get_context().get_partition_service().get_partition_count();
                    near_cache_ = nearCacheManager.get_or_create_near_cache<K, V, serialization::pimpl::data>(
                            spi::ClientProxy::get_name(), near_cache_config_);

                    near_cache_ = impl::nearcache::InvalidationAwareWrapper<
                            serialization::pimpl::data, V>::as_invalidation_aware(near_cache_, partitionCount);

                    key_state_marker_ = get_key_state_marker();

                    invalidate_on_change_ = near_cache_->is_invalidated_on_change();
                    if (invalidate_on_change_) {
                        std::shared_ptr<client::impl::BaseEventHandler> invalidationHandler(
                                new ClientMapAddNearCacheEventHandler(near_cache_));
                        add_near_cache_invalidate_listener(invalidationHandler);
                    }

                    local_map_stats_ = monitor::impl::LocalMapStatsImpl(near_cache_->get_near_cache_stats());
                }

                void post_destroy() override {
                    try {
                        remove_near_cache_invalidation_listener();
                        spi::ClientProxy::get_context().get_near_cache_manager().destroy_near_cache(
                                spi::ClientProxy::get_name());
                        spi::ClientProxy::post_destroy();
                    } catch (exception::iexception &) {
                        spi::ClientProxy::post_destroy();
                    }
                }

                void on_destroy() override {
                    remove_near_cache_invalidation_listener();
                    spi::ClientProxy::get_context().get_near_cache_manager().destroy_near_cache(spi::ClientProxy::get_name());

                    imap::on_shutdown();
                }

                boost::future<bool> contains_key_internal(const serialization::pimpl::data &key_data) override {
                    auto key = std::make_shared<serialization::pimpl::data>(key_data);
                    std::shared_ptr<V> cached = near_cache_->get(key);
                    if (cached) {
                        return boost::make_ready_future(internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached);
                    }

                    return imap::contains_key_internal(*key);
                }

                boost::future<boost::optional<serialization::pimpl::data>> get_internal(const serialization::pimpl::data &key_data) override {
                    auto key = std::make_shared<serialization::pimpl::data>(key_data);
                    auto cached = near_cache_->get(key);
                    if (cached) {
                        if (internal::nearcache::NearCache<K, V>::NULL_OBJECT == cached) {
                            return boost::make_ready_future(boost::optional<serialization::pimpl::data>());
                        }
                        return boost::make_ready_future(boost::make_optional(*cached));
                    }

                    bool marked = key_state_marker_->try_mark(*key);

                    try {
                        auto future = imap::get_internal(*key);
                        if (marked) {
                            return future.then(boost::launch::deferred, [=](boost::future<boost::optional<serialization::pimpl::data>> f) {
                                auto data = f.get();
                                auto cachedValue = data ? std::make_shared<serialization::pimpl::data>(*data)
                                                        : internal::nearcache::NearCache<K, V>::NULL_OBJECT;
                                try_to_put_near_cache(key, cachedValue);
                                return data;
                            });
                        }
                        return future;
                    } catch (exception::iexception &) {
                        reset_to_unmarked_state(key);
                        throw;
                    }
                }

                boost::future<bool> remove_internal(
                        const serialization::pimpl::data &key, const serialization::pimpl::data &value) override {
                    try {
                        auto response = imap::remove_internal(key, value);
                        invalidate_near_cache(key);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::data>> remove_internal(
                        const serialization::pimpl::data &key_data) override {
                    try {
                        invalidate_near_cache(key_data);
                        auto response = imap::remove_internal(key_data);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                remove_all_internal(const serialization::pimpl::data &predicate_data) override {
                    try {
                        auto response = imap::remove_all_internal(predicate_data);
                        near_cache_->clear();
                        return response;
                    } catch (exception::iexception &) {
                        near_cache_->clear();
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage> delete_internal(const serialization::pimpl::data &key) override {
                    try {
                        auto response = imap::delete_internal(key);
                        invalidate_near_cache(key);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key);
                        throw;
                    }
                }

                boost::future<bool> try_remove_internal(const serialization::pimpl::data &key_data,
                                                              std::chrono::milliseconds timeout) override {
                    try {
                        auto response = imap::try_remove_internal(key_data, timeout);
                        invalidate_near_cache(key_data);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<bool> try_put_internal(const serialization::pimpl::data &key_data,
                        const serialization::pimpl::data &value_data, std::chrono::milliseconds timeout) override {
                    try {
                        auto response = imap::try_put_internal(key_data, value_data, timeout);
                        invalidate_near_cache(key_data);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::data>> put_internal(const serialization::pimpl::data &key_data,
                        const serialization::pimpl::data &value_data, std::chrono::milliseconds ttl) override {
                    try {
                        auto previousValue = imap::put_internal(key_data, value_data, ttl);
                        invalidate_near_cache(key_data);
                        return previousValue;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                try_put_transient_internal(const serialization::pimpl::data &key_data,
                                        const serialization::pimpl::data &value_data,
                                        std::chrono::milliseconds ttl) override {
                    try {
                        auto result = imap::try_put_transient_internal(key_data, value_data, ttl);
                        invalidate_near_cache(key_data);
                        return result;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::data>>
                put_if_absent_internal(const serialization::pimpl::data &key_data,
                                    const serialization::pimpl::data &value_data,
                                    std::chrono::milliseconds ttl) override {
                    try {
                        auto previousValue = imap::put_if_absent_data(key_data, value_data, ttl);
                        invalidate_near_cache(key_data);
                        return previousValue;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<bool> replace_if_same_internal(const serialization::pimpl::data &key_data,
                                                          const serialization::pimpl::data &value_data,
                                                          const serialization::pimpl::data &new_value_data) override {
                    try {
                        auto result = proxy::IMapImpl::replace(key_data, value_data, new_value_data);
                        invalidate_near_cache(key_data);
                        return result;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::data>>
                replace_internal(const serialization::pimpl::data &key_data,
                                const serialization::pimpl::data &value_data) override {
                    try {
                        auto value = proxy::IMapImpl::replace_data(key_data, value_data);
                        invalidate_near_cache(key_data);
                        return value;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                set_internal(const serialization::pimpl::data &key_data, const serialization::pimpl::data &value_data,
                            std::chrono::milliseconds ttl) override {
                    try {
                        auto result = proxy::IMapImpl::set(key_data, value_data, ttl);
                        invalidate_near_cache(key_data);
                        return result;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<bool> evict_internal(const serialization::pimpl::data &key_data) override {
                    try {
                        auto evicted = proxy::IMapImpl::evict(key_data);
                        invalidate_near_cache(key_data);
                        return evicted;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<EntryVector>
                get_all_internal(int partition_id, const std::vector<serialization::pimpl::data> &partition_keys) override {
                    auto markers = std::make_shared<MARKER_MAP>();
                    try {
                        EntryVector result;
                        std::vector<serialization::pimpl::data> remainingKeys;
                        for (auto &key : partition_keys) {
                            auto key_data = std::make_shared<serialization::pimpl::data>(key);
                            auto cached = near_cache_->get(key_data);
                            if (cached && internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached) {
                                result.push_back(std::make_pair(std::move(*key_data), *cached));
                            } else {
                                if (invalidate_on_change_) {
                                    (*markers)[key_data] = key_state_marker_->try_mark(*key_data);
                                }
                                remainingKeys.push_back(std::move(*key_data));
                            }
                        }

                        if (remainingKeys.empty()) {
                            return boost::make_ready_future(result);
                        }

                        return imap::get_all_internal(partition_id, remainingKeys).then(
                                boost::launch::deferred, [=](boost::future<EntryVector> f) {
                            EntryVector allEntries(result);
                            for (auto &entry : f.get()) {
                                auto key = std::make_shared<serialization::pimpl::data>(std::move(entry.first));
                                auto value = std::make_shared<serialization::pimpl::data>(std::move(entry.second));
                                bool marked = false;
                                auto foundEntry = markers->find(key);
                                if (foundEntry != markers->end()) {
                                    marked = foundEntry->second;
                                    markers->erase(foundEntry);
                                }

                                if (marked) {
                                    try_to_put_near_cache(key, value);
                                } else {
                                    near_cache_->put(key, value);
                                }
                                allEntries.push_back(std::make_pair(std::move(*key), std::move(*value)));
                            }

                            unmark_remaining_marked_keys(*markers);
                            return allEntries;
                        });
                    } catch (exception::iexception &) {
                        unmark_remaining_marked_keys(*markers);
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::data>>
                execute_on_key_internal(const serialization::pimpl::data &key_data,
                                     const serialization::pimpl::data &processor) override {
                    try {
                        auto response = imap::execute_on_key_data(key_data, processor);
                        invalidate_near_cache(key_data);
                        return response;
                    } catch (exception::iexception &) {
                        invalidate_near_cache(key_data);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                put_all_internal(int partition_id, const EntryVector &entries) override {
                    try {
                        auto result = imap::put_all_internal(partition_id, entries);
                        invalidate_entries(entries);
                        return result;
                    } catch (exception::iexception &) {
                        invalidate_entries(entries);
                        throw;
                    }
                }

                void invalidate_entries(const EntryVector &entries) {
                    for (auto &entry : entries) {
                        invalidate_near_cache(std::make_shared<serialization::pimpl::data>(entry.first));
                    }
                }

            private:
                impl::nearcache::KeyStateMarker *get_key_state_marker() {
                    return std::static_pointer_cast<
                            impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::data, V> >(near_cache_)->
                            get_key_state_marker();
                }

                void add_near_cache_invalidate_listener(std::shared_ptr<client::impl::BaseEventHandler> &handler) {
                    try {
                        invalidation_listener_id_ = proxy::ProxyImpl::register_listener(create_near_cache_entry_listener_codec(),
                                                                                    handler).get();
                    } catch (exception::iexception &e) {
                        HZ_LOG(logger_, severe, 
                            boost::str(boost::format("Near Cache is not initialized!!! %1%") % e.what()) 
                        );
                    }
                }

                void remove_near_cache_invalidation_listener() {
                    if (invalidation_listener_id_.is_nil()) {
                        return;
                    }

                    proxy::ProxyImpl::deregister_listener(invalidation_listener_id_).get();
                }

                class ClientMapAddNearCacheEventHandler
                        : public protocol::codec::map_addnearcacheinvalidationlistener_handler {
                public:
                    // TODO: implement RepairingTask as in Java client
                    ClientMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::data, V> > &cache)
                            : near_cache_(cache) {
                    }

                    void before_listener_register() override {
                        near_cache_->clear();
                    }

                    void on_listener_register() override {
                        near_cache_->clear();
                    }

                    void handle_imapinvalidation(const boost::optional<data> &key, boost::uuids::uuid source_uuid,
                                                 boost::uuids::uuid partition_uuid, int64_t sequence) override {
                        // null key means Near Cache has to remove all entries in it (see MapAddNearCacheEntryListenerMessageTask)
                        if (!key) {
                            near_cache_->clear();
                        } else {
                            near_cache_->invalidate(std::make_shared<serialization::pimpl::data>(*key));
                        }
                    }

                    void handle_imapbatchinvalidation(const std::vector<serialization::pimpl::data> &keys,
                                                      const std::vector<boost::uuids::uuid> &source_uuids,
                                                      const std::vector<boost::uuids::uuid> &partition_uuids,
                                                      const std::vector<int64_t> &sequences) override {
                        for (auto &k : keys) {
                            near_cache_->invalidate(std::make_shared<serialization::pimpl::data>(k));
                        }
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::data, V> > near_cache_;
                };

                class NearCacheEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    protocol::ClientMessage encode_add_request(bool local_only) const override {
                        return protocol::codec::map_addnearcacheinvalidationlistener_encode(name_,
                                                                                                 static_cast<int32_t>(listener_flags_),
                                                                                                 local_only);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid real_registration_id) const override {
                        return protocol::codec::map_removeentrylistener_encode(name_, real_registration_id);
                    }

                    NearCacheEntryListenerMessageCodec(const std::string &name, entry_event::type listener_flags)
                            : name_(name), listener_flags_(listener_flags) {}

                private:
                    const std::string &name_;
                    entry_event::type listener_flags_;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_near_cache_entry_listener_codec() {
                    entry_event::type listenerFlags = entry_event::type::INVALIDATION;
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheEntryListenerMessageCodec(spi::ClientProxy::get_name(), listenerFlags));
                }

                void reset_to_unmarked_state(const std::shared_ptr<serialization::pimpl::data> &key) {
                    if (key_state_marker_->try_unmark(*key)) {
                        return;
                    }

                    invalidate_near_cache(key);
                    key_state_marker_->force_unmark(*key);
                }

                void unmark_remaining_marked_keys(MARKER_MAP &markers) {
                    for (MARKER_MAP::const_iterator it = markers.begin();
                         it != markers.end(); ++it) {
                        if (it->second) {
                            key_state_marker_->force_unmark(*it->first);
                        }
                    }
                }

                void try_to_put_near_cache(const std::shared_ptr<serialization::pimpl::data> &key_data,
                                       const std::shared_ptr<V> &response) {
                    try_to_put_near_cache_internal<V>(key_data, response);
                }

/*
                void tryToPutNearCache(const std::shared_ptr<serialization::pimpl::data> &key_data,
                                       const std::shared_ptr<serialization::pimpl::data> &response) {
                    tryToPutNearCacheInternal<serialization::pimpl::data>(key_data, response);
                }
*/

                template<typename VALUETYPE>
                void try_to_put_near_cache_internal(const std::shared_ptr<serialization::pimpl::data> &key_data,
                                               const std::shared_ptr<VALUETYPE> &response) {
                    try {
                        near_cache_->put(key_data, response);
                        reset_to_unmarked_state(key_data);
                    } catch (exception::iexception &) {
                        reset_to_unmarked_state(key_data);
                        throw;
                    }
                }

                /**
                 * This method modifies the key data internal pointer although it is marked as const
                 * @param key The key for which to invalidate the near cache
                 */
                void invalidate_near_cache(const serialization::pimpl::data &key) {
                    near_cache_->invalidate(std::make_shared<serialization::pimpl::data>(key));
                }

                void invalidate_near_cache(std::shared_ptr<serialization::pimpl::data> key) {
                    near_cache_->invalidate(key);
                }

                bool cache_local_entries_;
                bool invalidate_on_change_;
                impl::nearcache::KeyStateMarker *key_state_marker_;
                const config::near_cache_config &near_cache_config_;
                std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::data, V>> near_cache_;
                boost::uuids::uuid invalidation_listener_id_;
                logger &logger_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

