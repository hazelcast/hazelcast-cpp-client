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

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ReplicatedMapImpl : public ProxyImpl {
            public:
                /**
                 *
                 * @return The number of the replicated map entries in the cluster.
                 */
                boost::future<int32_t> size() {
                    auto request = protocol::codec::replicatedmap_size_encode(name_);
                    return invoke_and_get_future<int32_t>(
                            request, targetPartitionId_);
                }

                /**
                 *
                 * @return true if the replicated map is empty, false otherwise
                 */
                boost::future<bool> is_empty() {
                    auto request = protocol::codec::replicatedmap_isempty_encode(name_);
                    return invoke_and_get_future<bool>(
                            request, targetPartitionId_);
                }

                /**
                 * <p>The clear operation wipes data out of the replicated maps.
                 * <p>If some node fails on executing the operation, it is retried for at most
                 * 5 times (on the failing nodes only).
                 */
                boost::future<void> clear() {
                    try {
                        auto request = protocol::codec::replicatedmap_clear_encode(name_);
                        auto result = to_void_future(invoke(request));
                        if (nearCache_) {
                            nearCache_->clear();
                        }
                        return result;
                    } catch (...) {
                        if (nearCache_) {
                            nearCache_->clear();
                        }
                        throw;
                    }
                }

                /**
                 * Removes the specified entry listener.
                 * Returns silently if there was no such listener added before.
                 *
                 * @param registrationId ID of the registered entry listener.
                 * @return true if registration is removed, false otherwise.
                 */
                boost::future<bool> remove_entry_listener(boost::uuids::uuid registrationId) {
                    return deregister_listener(registrationId);
                }

            protected:
                boost::future<boost::optional<serialization::pimpl::Data>>
                put_data(serialization::pimpl::Data &&keyData, serialization::pimpl::Data &&valueData,
                    std::chrono::milliseconds ttl) {
                    try {
                        auto request = protocol::codec::replicatedmap_put_encode(name_, keyData, valueData,
                                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                                     ttl).count());
                        auto result = invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                                request, keyData);

                        invalidate(std::move(keyData));

                        return result;
                    } catch (...) {
                        invalidate(std::move(keyData));
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>>remove_data(serialization::pimpl::Data &&keyData) {
                    std::shared_ptr<serialization::pimpl::Data> sharedKey(new serialization::pimpl::Data(std::move(keyData)));
                    try {
                        auto request = protocol::codec::replicatedmap_remove_encode(name_, *sharedKey);
                        auto result = invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                                request, *sharedKey);

                        invalidate(sharedKey);

                        return result;
                    } catch (...) {
                        invalidate(sharedKey);
                        throw;
                    }
                }

                virtual boost::future<void> put_all_data(EntryVector &&dataEntries) {
                    try {
                        auto request = protocol::codec::replicatedmap_putall_encode(name_, dataEntries);
                        auto result = to_void_future(invoke(request));

                        if (nearCache_) {
                            for (auto &entry : dataEntries) {
                                invalidate(std::move(entry.first));
                            }
                        }

                        return result;
                    } catch (...) {
                        if (nearCache_) {
                            for (auto &entry : dataEntries) {
                                invalidate(std::move(entry.first));
                            }
                        }
                        throw;
                    }
                }

                boost::future<bool> contains_key_data(serialization::pimpl::Data &&keyData) {
                    auto request = protocol::codec::replicatedmap_containskey_encode(name_, keyData);
                    return invoke_and_get_future<bool>(
                            request, keyData);
                }

                boost::future<bool> contains_value_data(serialization::pimpl::Data &&valueData) {
                    auto request = protocol::codec::replicatedmap_containsvalue_encode(name_, valueData);
                    return invoke_and_get_future<bool>(
                            request, valueData);
                }

                boost::future<boost::uuids::uuid> add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler) {
                    return register_listener(create_entry_listener_codec(get_name()), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> add_entry_listener_to_key(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                         serialization::pimpl::Data &&key) {
                    return register_listener(create_entry_listener_to_key_codec(std::move(key)), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                    serialization::pimpl::Data &&predicate) {
                    return register_listener(create_entry_listener_with_predicate_codec(std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                    serialization::pimpl::Data &&key, serialization::pimpl::Data &&predicate) {
                    return register_listener(create_entry_listener_to_key_with_predicate_codec(std::move(key), std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<std::vector<serialization::pimpl::Data>> values_data() {
                    auto request = protocol::codec::replicatedmap_values_encode(get_name());
                    return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                            request, targetPartitionId_);
                }

                boost::future<EntryVector> entry_set_data() {
                    auto request = protocol::codec::replicatedmap_entryset_encode(get_name());
                    return invoke_and_get_future<EntryVector>(request, targetPartitionId_);
                }

                boost::future<std::vector<serialization::pimpl::Data>> key_set_data() {
                    auto request = protocol::codec::replicatedmap_keyset_encode(get_name());
                    return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                            request, targetPartitionId_);
                }

                boost::future<boost::optional<serialization::pimpl::Data>> get_data(serialization::pimpl::Data &&key) {
                    auto sharedKey = std::make_shared<serialization::pimpl::Data>(key);
                    auto cachedValue = get_cached_data(sharedKey);
                    if (cachedValue) {
                        return boost::make_ready_future(boost::make_optional(*cachedValue));
                    }
                    auto request = protocol::codec::replicatedmap_get_encode(get_name(), *sharedKey);
                    return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                            request, key).then(boost::launch::deferred, [=] (boost::future<boost::optional<serialization::pimpl::Data>> f) {
                                try {
                                    auto response = f.get();
                                    if (!response) {
                                        return boost::optional<serialization::pimpl::Data>();
                                    }

                                    auto sharedValue = std::make_shared<serialization::pimpl::Data>(std::move(*response));
                                    if (nearCache_) {
                                        nearCache_->put(sharedKey, sharedValue);
                                    }
                                    return boost::make_optional(*sharedValue);
                                } catch (...) {
                                    invalidate(sharedKey);
                                    throw;
                                }
                            });
                }

                std::shared_ptr<serialization::pimpl::Data> get_cached_data(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (!nearCache_) {
                        return nullptr;
                    }
                    return nearCache_->get(key);
                }

                void on_initialize() override {
                    ProxyImpl::on_initialize();

                    int partitionCount = get_context().get_partition_service().get_partition_count();
                    targetPartitionId_ = rand() % partitionCount;

                    init_near_cache();
                }

                void post_destroy() override  {
                    try {
                        if (nearCache_) {
                            remove_near_cache_invalidation_listener();
                            get_context().get_near_cache_manager().destroy_near_cache(name_);
                        }

                        ProxyImpl::post_destroy();
                    } catch (...) {
                        ProxyImpl::post_destroy();
                        throw;
                    }
                }

                ReplicatedMapImpl(const std::string &serviceName, const std::string &objectName,
                                  spi::ClientContext *context);
            private:
                int targetPartitionId_;
                std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache_;
                boost::uuids::uuid invalidationListenerId_;
                
                class NearCacheInvalidationListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    NearCacheInvalidationListenerMessageCodec(const std::string &name) : name_(name) {}

                    protocol::ClientMessage encode_add_request(bool localOnly) const override{
                        return protocol::codec::replicatedmap_addnearcacheentrylistener_encode(name_, false, localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override{
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name_, realRegistrationId);
                    }

                private:
                    std::string name_;
                };

                class ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec
                        : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(const std::string &name,
                                                                                serialization::pimpl::Data &&keyData,
                                                                                serialization::pimpl::Data &&predicateData)
                            : name_(name), keyData_(keyData), predicateData_(predicateData) {}

                    protocol::ClientMessage encode_add_request(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenertokeywithpredicate_encode(
                                name_, keyData_, predicateData_, localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override{
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name_, realRegistrationId);
                    }

                private:
                    std::string name_;
                    serialization::pimpl::Data keyData_;
                    serialization::pimpl::Data predicateData_;
                };

                class ReplicatedMapAddEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerWithPredicateMessageCodec(const std::string &name,
                                                                          serialization::pimpl::Data &&keyData)
                            : name_(name), predicateData_(keyData) {}

                    protocol::ClientMessage encode_add_request(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenerwithpredicate_encode(name_, predicateData_, localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name_, realRegistrationId);
                    }

                private:
                    std::string name_;
                    serialization::pimpl::Data predicateData_;
                };

                class ReplicatedMapAddEntryListenerToKeyMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyMessageCodec(const std::string &name,
                                                                   serialization::pimpl::Data &&keyData) : name_(name), keyData_(keyData) {}

                    protocol::ClientMessage encode_add_request(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenertokey_encode(name_, keyData_, localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name_, realRegistrationId);
                    }

                private:
                    std::string name_;
                    serialization::pimpl::Data keyData_;
                };

                class ReplicatedMapListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapListenerMessageCodec(const std::string &name) : name_(name) {}

                    protocol::ClientMessage encode_add_request(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistener_encode(name_, localOnly);
                    }

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name_, realRegistrationId);
                    }

                private:
                    std::string name_;
                };

                class ReplicatedMapAddNearCacheEventHandler
                        : public protocol::codec::replicatedmap_addnearcacheentrylistener_handler {
                public:
                    ReplicatedMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> &nearCache) : nearCache_(nearCache) {}

                    void before_listener_register() override {
                        nearCache_->clear();
                    }

                    void on_listener_register() override {
                        nearCache_->clear();
                    }

                    void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value,
                                      const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue,
                                      int32_t eventType, boost::uuids::uuid uuid,
                                      int32_t numberOfAffectedEntries) override {
                        switch (eventType) {
                            case static_cast<int32_t>(EntryEvent::type::ADDED):
                            case static_cast<int32_t>(EntryEvent::type::REMOVED):
                            case static_cast<int32_t>(EntryEvent::type::UPDATED):
                            case static_cast<int32_t>(EntryEvent::type::EVICTED): {
                                nearCache_->invalidate(std::make_shared<serialization::pimpl::Data>(std::move(*key)));
                                break;
                            }
                            case static_cast<int32_t>(EntryEvent::type::CLEAR_ALL):
                                nearCache_->clear();
                                break;
                            default:
                                BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                        "ReplicatedMapAddNearCacheEventHandler::handle_entry")
                                        << "Not a known event type " << eventType).build());
                        }
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache_;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_entry_listener_codec(const std::string name) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapListenerMessageCodec(name));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_entry_listener_to_key_codec(serialization::pimpl::Data &&keyData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyMessageCodec(name_, std::move(keyData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_entry_listener_with_predicate_codec(serialization::pimpl::Data &&predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerWithPredicateMessageCodec(name_, std::move(predicateData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_entry_listener_to_key_with_predicate_codec(serialization::pimpl::Data &&keyData,
                                                           serialization::pimpl::Data &&predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(name_, std::move(keyData),
                                                                                            std::move(predicateData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_near_cache_invalidation_listener_codec() {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheInvalidationListenerMessageCodec(name_));
                }

                void register_invalidation_listener() {
                    try {
                        invalidationListenerId_ = register_listener(create_near_cache_invalidation_listener_codec(),
                                                                  std::shared_ptr<impl::BaseEventHandler>(new ReplicatedMapAddNearCacheEventHandler(nearCache_))).get();
                    } catch (exception::IException &e) {
                        HZ_LOG(get_context().get_logger(), severe,
                            boost::str(boost::format("-----------------\n"
                                                     "Near Cache is not initialized!\n"
                                                     "-----------------"
                                                     "%1%") % e)
                        );
                    }
                }

                void init_near_cache() {
                    auto nearCacheConfig = get_context().get_client_config().get_near_cache_config(name_);
                    if (nearCacheConfig) {
                        nearCache_ = get_context().get_near_cache_manager().template get_or_create_near_cache<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(
                                name_, *nearCacheConfig);
                        if (nearCacheConfig->is_invalidate_on_change()) {
                            register_invalidation_listener();
                        }
                    }

                }

                void remove_near_cache_invalidation_listener() {
                    if (nearCache_) {
                        if (!invalidationListenerId_.is_nil()) {
                            deregister_listener(invalidationListenerId_).get();
                        }
                    }
                }

                void invalidate(serialization::pimpl::Data &&key) {
                    if (!nearCache_) {
                        return;
                    }
                    nearCache_->invalidate(std::make_shared<serialization::pimpl::Data>(key));
                }

                void invalidate(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (!nearCache_) {
                        return;
                    }
                    nearCache_->invalidate(key);
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
