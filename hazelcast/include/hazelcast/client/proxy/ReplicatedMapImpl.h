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
                    auto request = protocol::codec::replicatedmap_size_encode(name);
                    return invokeAndGetFuture<int32_t>(
                            request, targetPartitionId);
                }

                /**
                 *
                 * @return true if the replicated map is empty, false otherwise
                 */
                boost::future<bool> isEmpty() {
                    auto request = protocol::codec::replicatedmap_isempty_encode(name);
                    return invokeAndGetFuture<bool>(
                            request, targetPartitionId);
                }

                /**
                 * <p>The clear operation wipes data out of the replicated maps.
                 * <p>If some node fails on executing the operation, it is retried for at most
                 * 5 times (on the failing nodes only).
                 */
                boost::future<void> clear() {
                    try {
                        auto request = protocol::codec::replicatedmap_clear_encode(name);
                        auto result = toVoidFuture(invoke(request));
                        if (nearCache) {
                            nearCache->clear();
                        }
                        return result;
                    } catch (...) {
                        if (nearCache) {
                            nearCache->clear();
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
                boost::future<bool> removeEntryListener(boost::uuids::uuid registrationId) {
                    return deregisterListener(registrationId);
                }

            protected:
                boost::future<boost::optional<serialization::pimpl::Data>>
                putData(serialization::pimpl::Data &&keyData, serialization::pimpl::Data &&valueData,
                    std::chrono::steady_clock::duration ttl) {
                    try {
                        auto request = protocol::codec::replicatedmap_put_encode(name, keyData, valueData,
                                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                                     ttl).count());
                        auto result = invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                                request, keyData);

                        invalidate(std::move(keyData));

                        return result;
                    } catch (...) {
                        invalidate(std::move(keyData));
                        throw;
                    }
                }

                boost::future<boost::optional<serialization::pimpl::Data>>removeData(serialization::pimpl::Data &&keyData) {
                    std::shared_ptr<serialization::pimpl::Data> sharedKey(new serialization::pimpl::Data(std::move(keyData)));
                    try {
                        auto request = protocol::codec::replicatedmap_remove_encode(name, *sharedKey);
                        auto result = invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                                request, *sharedKey);

                        invalidate(sharedKey);

                        return result;
                    } catch (...) {
                        invalidate(sharedKey);
                        throw;
                    }
                }

                virtual boost::future<void> putAllData(EntryVector &&dataEntries) {
                    try {
                        auto request = protocol::codec::replicatedmap_putall_encode(name, dataEntries);
                        auto result = toVoidFuture(invoke(request));

                        if (nearCache) {
                            for (auto &entry : dataEntries) {
                                invalidate(std::move(entry.first));
                            }
                        }

                        return result;
                    } catch (...) {
                        if (nearCache) {
                            for (auto &entry : dataEntries) {
                                invalidate(std::move(entry.first));
                            }
                        }
                        throw;
                    }
                }

                boost::future<bool> containsKeyData(serialization::pimpl::Data &&keyData) {
                    auto request = protocol::codec::replicatedmap_containskey_encode(name, keyData);
                    return invokeAndGetFuture<bool>(
                            request, keyData);
                }

                boost::future<bool> containsValueData(serialization::pimpl::Data &&valueData) {
                    auto request = protocol::codec::replicatedmap_containsvalue_encode(name, valueData);
                    return invokeAndGetFuture<bool>(
                            request, valueData);
                }

                boost::future<boost::uuids::uuid> addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler) {
                    return registerListener(createEntryListenerCodec(getName()), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> addEntryListenerToKey(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                         serialization::pimpl::Data &&key) {
                    return registerListener(createEntryListenerToKeyCodec(std::move(key)), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                    serialization::pimpl::Data &&predicate) {
                    return registerListener(createEntryListenerWithPredicateCodec(std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<boost::uuids::uuid> addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler,
                                                                                    serialization::pimpl::Data &&key, serialization::pimpl::Data &&predicate) {
                    return registerListener(createEntryListenerToKeyWithPredicateCodec(std::move(key), std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<std::vector<serialization::pimpl::Data>> valuesData() {
                    auto request = protocol::codec::replicatedmap_values_encode(getName());
                    return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                            request, targetPartitionId);
                }

                boost::future<EntryVector> entrySetData() {
                    auto request = protocol::codec::replicatedmap_entryset_encode(getName());
                    return invokeAndGetFuture<EntryVector>(request, targetPartitionId);
                }

                boost::future<std::vector<serialization::pimpl::Data>> keySetData() {
                    auto request = protocol::codec::replicatedmap_keyset_encode(getName());
                    return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                            request, targetPartitionId);
                }

                boost::future<boost::optional<serialization::pimpl::Data>> getData(serialization::pimpl::Data &&key) {
                    auto sharedKey = std::make_shared<serialization::pimpl::Data>(key);
                    auto cachedValue = getCachedData(sharedKey);
                    if (cachedValue) {
                        return boost::make_ready_future(boost::make_optional(*cachedValue));
                    }
                    auto request = protocol::codec::replicatedmap_get_encode(getName(), *sharedKey);
                    return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                            request, key).then(boost::launch::deferred, [=] (boost::future<boost::optional<serialization::pimpl::Data>> f) {
                                try {
                                    auto response = f.get();
                                    if (!response) {
                                        return boost::optional<serialization::pimpl::Data>();
                                    }

                                    auto sharedValue = std::make_shared<serialization::pimpl::Data>(std::move(*response));
                                    if (nearCache) {
                                        nearCache->put(sharedKey, sharedValue);
                                    }
                                    return boost::make_optional(*sharedValue);
                                } catch (...) {
                                    invalidate(sharedKey);
                                    throw;
                                }
                            });
                }

                std::shared_ptr<serialization::pimpl::Data> getCachedData(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (!nearCache) {
                        return nullptr;
                    }
                    return nearCache->get(key);
                }

                void onInitialize() override {
                    ProxyImpl::onInitialize();

                    int partitionCount = getContext().getPartitionService().getPartitionCount();
                    targetPartitionId = rand() % partitionCount;

                    initNearCache();
                }

                void postDestroy() override  {
                    try {
                        if (nearCache) {
                            removeNearCacheInvalidationListener();
                            getContext().getNearCacheManager().destroyNearCache(name);
                        }

                        ProxyImpl::postDestroy();
                    } catch (...) {
                        ProxyImpl::postDestroy();
                        throw;
                    }
                }

                ReplicatedMapImpl(const std::string &serviceName, const std::string &objectName,
                                  spi::ClientContext *context);
            private:
                int targetPartitionId;
                std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache;
                boost::uuids::uuid invalidationListenerId;
                
                class NearCacheInvalidationListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    NearCacheInvalidationListenerMessageCodec(const std::string &name) : name(name) {}

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override{
                        return protocol::codec::replicatedmap_addnearcacheentrylistener_encode(name, false, localOnly);
                    }

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override{
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name, realRegistrationId);
                    }

                private:
                    std::string name;
                };

                class ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec
                        : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(const std::string &name,
                                                                                serialization::pimpl::Data &&keyData,
                                                                                serialization::pimpl::Data &&predicateData)
                            : name(name), keyData(keyData), predicateData(predicateData) {}

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenertokeywithpredicate_encode(
                                name, keyData, predicateData, localOnly);
                    }

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override{
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name, realRegistrationId);
                    }

                private:
                    std::string name;
                    serialization::pimpl::Data keyData;
                    serialization::pimpl::Data predicateData;
                };

                class ReplicatedMapAddEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerWithPredicateMessageCodec(const std::string &name,
                                                                          serialization::pimpl::Data &&keyData)
                            : name(name), predicateData(keyData) {}

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenerwithpredicate_encode(name, predicateData, localOnly);
                    }

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name, realRegistrationId);
                    }

                private:
                    std::string name;
                    serialization::pimpl::Data predicateData;
                };

                class ReplicatedMapAddEntryListenerToKeyMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyMessageCodec(const std::string &name,
                                                                   serialization::pimpl::Data &&keyData) : name(name), keyData(keyData) {}

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistenertokey_encode(name, keyData, localOnly);
                    }

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name, realRegistrationId);
                    }

                private:
                    std::string name;
                    serialization::pimpl::Data keyData;
                };

                class ReplicatedMapListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapListenerMessageCodec(const std::string &name) : name(name) {}

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::replicatedmap_addentrylistener_encode(name, localOnly);
                    }

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override {
                        return protocol::codec::replicatedmap_removeentrylistener_encode(name, realRegistrationId);
                    }

                private:
                    std::string name;
                };

                class ReplicatedMapAddNearCacheEventHandler
                        : public protocol::codec::replicatedmap_addnearcacheentrylistener_handler {
                public:
                    ReplicatedMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> &nearCache) : nearCache(nearCache) {}

                    void beforeListenerRegister() override {
                        nearCache->clear();
                    }

                    void onListenerRegister() override {
                        nearCache->clear();
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
                                nearCache->invalidate(std::make_shared<serialization::pimpl::Data>(std::move(*key)));
                                break;
                            }
                            case static_cast<int32_t>(EntryEvent::type::CLEAR_ALL):
                                nearCache->clear();
                                break;
                            default:
                                BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                        "ReplicatedMapAddNearCacheEventHandler::handle_entry")
                                        << "Not a known event type " << eventType).build());
                        }
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec> createEntryListenerCodec(const std::string name) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapListenerMessageCodec(name));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyCodec(serialization::pimpl::Data &&keyData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyMessageCodec(name, std::move(keyData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerWithPredicateCodec(serialization::pimpl::Data &&predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerWithPredicateMessageCodec(name, std::move(predicateData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyWithPredicateCodec(serialization::pimpl::Data &&keyData,
                                                           serialization::pimpl::Data &&predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(name, std::move(keyData),
                                                                                            std::move(predicateData)));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec> createNearCacheInvalidationListenerCodec() {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheInvalidationListenerMessageCodec(name));
                }

                void registerInvalidationListener() {
                    try {
                        invalidationListenerId = registerListener(createNearCacheInvalidationListenerCodec(),
                                                                  std::shared_ptr<impl::BaseEventHandler>(new ReplicatedMapAddNearCacheEventHandler(nearCache))).get();
                    } catch (exception::IException &e) {
                        HZ_LOG(getContext().getLogger(), severe,
                            boost::str(boost::format("-----------------\n"
                                                     "Near Cache is not initialized!\n"
                                                     "-----------------"
                                                     "%1%") % e)
                        );
                    }
                }

                void initNearCache() {
                    auto nearCacheConfig = getContext().getClientConfig().template getNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(name);
                    if (nearCacheConfig.get() != NULL) {
                        nearCache = getContext().getNearCacheManager().template getOrCreateNearCache<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(
                                name, *nearCacheConfig);
                        if (nearCacheConfig->isInvalidateOnChange()) {
                            registerInvalidationListener();
                        }
                    }

                }

                void removeNearCacheInvalidationListener() {
                    if (nearCache) {
                        if (!invalidationListenerId.is_nil()) {
                            deregisterListener(invalidationListenerId).get();
                        }
                    }
                }

                void invalidate(serialization::pimpl::Data &&key) {
                    if (!nearCache) {
                        return;
                    }
                    nearCache->invalidate(std::make_shared<serialization::pimpl::Data>(key));
                }

                void invalidate(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (!nearCache) {
                        return;
                    }
                    nearCache->invalidate(key);
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
