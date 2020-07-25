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
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

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
                    auto request = protocol::codec::ReplicatedMapSizeCodec::encodeRequest(name);
                    return invokeAndGetFuture<int32_t, protocol::codec::ReplicatedMapSizeCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                /**
                 *
                 * @return true if the replicated map is empty, false otherwise
                 */
                boost::future<bool> isEmpty() {
                    auto request = protocol::codec::ReplicatedMapIsEmptyCodec::encodeRequest(name);
                    return invokeAndGetFuture<bool, protocol::codec::ReplicatedMapIsEmptyCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                /**
                 * <p>The clear operation wipes data out of the replicated maps.
                 * <p>If some node fails on executing the operation, it is retried for at most
                 * 5 times (on the failing nodes only).
                 */
                boost::future<void> clear() {
                    try {
                        auto request = protocol::codec::ReplicatedMapClearCodec::encodeRequest(name);
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
                boost::future<bool> removeEntryListener(const std::string &registrationId) {
                    return deregisterListener(registrationId);
                }

            protected:
                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                putData(serialization::pimpl::Data &&keyData, serialization::pimpl::Data &&valueData,
                    std::chrono::steady_clock::duration ttl) {
                    try {
                        auto request = protocol::codec::ReplicatedMapPutCodec::encodeRequest(name, keyData, valueData,
                                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                                     ttl).count());
                        auto result = invokeAndGetFuture<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapPutCodec::ResponseParameters>(
                                request, keyData);

                        invalidate(std::move(keyData));

                        return result;
                    } catch (...) {
                        invalidate(std::move(keyData));
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>> removeData(serialization::pimpl::Data &&keyData) {
                    std::shared_ptr<serialization::pimpl::Data> sharedKey(new serialization::pimpl::Data(std::move(keyData)));
                    try {
                        auto request = protocol::codec::ReplicatedMapRemoveCodec::encodeRequest(name, *sharedKey);
                        auto result = invokeAndGetFuture<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapRemoveCodec::ResponseParameters>(
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
                        auto request = protocol::codec::ReplicatedMapPutAllCodec::encodeRequest(name, dataEntries);
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
                    auto request = protocol::codec::ReplicatedMapContainsKeyCodec::encodeRequest(name, keyData);
                    return invokeAndGetFuture<bool, protocol::codec::ReplicatedMapContainsKeyCodec::ResponseParameters>(
                            request, keyData);
                }

                boost::future<bool> containsValueData(serialization::pimpl::Data &&valueData) {
                    auto request = protocol::codec::ReplicatedMapContainsValueCodec::encodeRequest(name, valueData);
                    return invokeAndGetFuture<bool, protocol::codec::ReplicatedMapContainsValueCodec::ResponseParameters>(
                            request, valueData);
                }

                boost::future<std::string> addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler) {
                    return registerListener(createEntryListenerCodec(getName()), std::move(entryEventHandler));
                }

                boost::future<std::string> addEntryListenerToKey(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler,
                        serialization::pimpl::Data &&key) {
                    return registerListener(createEntryListenerToKeyCodec(std::move(key)), std::move(entryEventHandler));
                }

                boost::future<std::string> addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler,
                        serialization::pimpl::Data &&predicate) {
                    return registerListener(createEntryListenerWithPredicateCodec(std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<std::string> addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler,
                                                            serialization::pimpl::Data &&key, serialization::pimpl::Data &&predicate) {
                    return registerListener(createEntryListenerToKeyWithPredicateCodec(std::move(key), std::move(predicate)), std::move(entryEventHandler));
                }

                boost::future<std::vector<serialization::pimpl::Data>> valuesData() {
                    auto request = protocol::codec::ReplicatedMapValuesCodec::encodeRequest(getName());
                    return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::ReplicatedMapValuesCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                boost::future<EntryVector> entrySetData() {
                    auto request = protocol::codec::ReplicatedMapEntrySetCodec::encodeRequest(getName());
                    return invokeAndGetFuture<EntryVector, protocol::codec::ReplicatedMapEntrySetCodec::ResponseParameters>(request, targetPartitionId);
                }

                boost::future<std::vector<serialization::pimpl::Data>> keySetData() {
                    auto request = protocol::codec::ReplicatedMapKeySetCodec::encodeRequest(getName());
                    return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::ReplicatedMapKeySetCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                boost::future<boost::optional<serialization::pimpl::Data>> getData(serialization::pimpl::Data &&key) {
                    auto sharedKey = std::make_shared<serialization::pimpl::Data>(key);
                    auto cachedValue = getCachedData(sharedKey);
                    if (cachedValue) {
                        return boost::make_ready_future(boost::make_optional(*cachedValue));
                    }
                    auto request = protocol::codec::ReplicatedMapGetCodec::encodeRequest(getName(), *sharedKey);
                    return invokeAndGetFuture<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapGetCodec::ResponseParameters>(
                            request, key).then(boost::launch::deferred, [=] (boost::future<std::unique_ptr<serialization::pimpl::Data>> f) {
                                try {
                                    auto response = f.get();
                                    if (!response) {
                                        return boost::optional<serialization::pimpl::Data>();
                                    }

                                    std::shared_ptr<serialization::pimpl::Data> sharedValue(std::move(response));
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
                std::string invalidationListenerId;
                
                class NearCacheInvalidationListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    NearCacheInvalidationListenerMessageCodec(const std::string &name) : name(name) {}

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::encodeRequest(name, false,
                                                                                                           localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override {
                        return protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
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

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::encodeRequest(
                                name, keyData, predicateData, localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
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

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerWithPredicateCodec::encodeRequest(name,
                                                                                                               predicateData,
                                                                                                               localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                    serialization::pimpl::Data predicateData;
                };

                class ReplicatedMapAddEntryListenerToKeyMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyMessageCodec(const std::string &name,
                                                                   serialization::pimpl::Data &&keyData) : name(name), keyData(keyData) {}

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyCodec::encodeRequest(name, keyData,
                                                                                                       localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                    serialization::pimpl::Data keyData;
                };

                class ReplicatedMapListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapListenerMessageCodec(const std::string &name) : name(name) {}

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerCodec::encodeRequest(name, localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override {
                        return protocol::codec::ReplicatedMapAddEntryListenerCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                };

                class ReplicatedMapAddNearCacheEventHandler
                        : public protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler {
                public:
                    ReplicatedMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> &nearCache) : nearCache(nearCache) {}

                    void beforeListenerRegister() override {
                        nearCache->clear();
                    }

                    void onListenerRegister() override {
                        nearCache->clear();
                    }

                    void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &dataKey,
                                             std::unique_ptr<serialization::pimpl::Data> &value,
                                             std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                             std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                             const int32_t &eventType, const std::string &uuid,
                                             const int32_t &numberOfAffectedEntries) override {
                        switch (eventType) {
                            case static_cast<int32_t>(EntryEvent::type::ADDED):
                            case static_cast<int32_t>(EntryEvent::type::REMOVED):
                            case static_cast<int32_t>(EntryEvent::type::UPDATED):
                            case static_cast<int32_t>(EntryEvent::type::EVICTED): {
                                nearCache->invalidate(std::shared_ptr<serialization::pimpl::Data>(std::move(dataKey)));
                                break;
                            }
                            case static_cast<int32_t>(EntryEvent::type::CLEAR_ALL):
                                nearCache->clear();
                                break;
                            default:
                                BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                        "ReplicatedMapAddNearCacheEventHandler::handleEntryEventV10")
                                        << "Not a known event type " << eventType).build());
                        }
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache;
                };

                std::unique_ptr<spi::impl::ListenerMessageCodec> createEntryListenerCodec(const std::string name) {
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapListenerMessageCodec(name));
                }

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyCodec(serialization::pimpl::Data &&keyData) {
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyMessageCodec(name, std::move(keyData)));
                }

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerWithPredicateCodec(serialization::pimpl::Data &&predicateData) {
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerWithPredicateMessageCodec(name, std::move(predicateData)));
                }

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyWithPredicateCodec(serialization::pimpl::Data &&keyData,
                                                           serialization::pimpl::Data &&predicateData) {
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(name, std::move(keyData),
                                                                                            std::move(predicateData)));
                }

                std::unique_ptr<spi::impl::ListenerMessageCodec> createNearCacheInvalidationListenerCodec() {
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheInvalidationListenerMessageCodec(name));
                }

                void registerInvalidationListener() {
                    try {
                        invalidationListenerId = registerListener(createNearCacheInvalidationListenerCodec(),
                                                                  std::unique_ptr<impl::BaseEventHandler>(new ReplicatedMapAddNearCacheEventHandler(nearCache))).get();
                    } catch (exception::IException &e) {
                        getContext().getLogger().severe("-----------------\nNear Cache is not initialized!\n-----------------" , e);
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
                        deregisterListener(invalidationListenerId).get();
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
