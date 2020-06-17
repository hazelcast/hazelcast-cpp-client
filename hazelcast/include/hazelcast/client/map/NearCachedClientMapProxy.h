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
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"

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
                        : IMap(instanceName, context), cacheLocalEntries(false),
                          invalidateOnChange(false), keyStateMarker(NULL),
                          nearCacheConfig(*context->getClientConfig().getNearCacheConfig<K, V>(instanceName)),
                          logger(context->getLogger()) {}

            protected:
                typedef std::unordered_map<std::shared_ptr<serialization::pimpl::Data>, bool> MARKER_MAP;

                void onInitialize() override {
                    IMap::onInitialize();

                    internal::nearcache::NearCacheManager &nearCacheManager = this->getContext().getNearCacheManager();
                    cacheLocalEntries = nearCacheConfig.isCacheLocalEntries();
                    int partitionCount = this->getContext().getPartitionService().getPartitionCount();
                    nearCache = nearCacheManager.getOrCreateNearCache<K, V, serialization::pimpl::Data>(
                            spi::ClientProxy::getName(), nearCacheConfig);

                    nearCache = impl::nearcache::InvalidationAwareWrapper<
                            serialization::pimpl::Data, V>::asInvalidationAware(nearCache, partitionCount);

                    keyStateMarker = getKeyStateMarker();

                    invalidateOnChange = nearCache->isInvalidatedOnChange();
                    if (invalidateOnChange) {
                        std::unique_ptr<client::impl::BaseEventHandler> invalidationHandler(
                                new ClientMapAddNearCacheEventHandler(nearCache));
                        addNearCacheInvalidateListener(invalidationHandler);
                    }

                    localMapStats = monitor::impl::LocalMapStatsImpl(nearCache->getNearCacheStats());
                }

                void postDestroy() override {
                    try {
                        removeNearCacheInvalidationListener();
                        spi::ClientProxy::getContext().getNearCacheManager().destroyNearCache(
                                spi::ClientProxy::getName());
                        spi::ClientProxy::postDestroy();
                    } catch (exception::IException &) {
                        spi::ClientProxy::postDestroy();
                    }
                }

                void onDestroy() override {
                    removeNearCacheInvalidationListener();
                    spi::ClientProxy::getContext().getNearCacheManager().destroyNearCache(spi::ClientProxy::getName());

                    IMap::onShutdown();
                }

                boost::future<bool> containsKeyInternal(const serialization::pimpl::Data &keyData) override {
                    auto key = std::make_shared<serialization::pimpl::Data>(keyData);
                    std::shared_ptr<V> cached = nearCache->get(key);
                    if (cached) {
                        return boost::make_ready_future(internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached);
                    }

                    return IMap::containsKeyInternal(*key);
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>> getInternal(const serialization::pimpl::Data &keyData) override {
                    auto key = std::make_shared<serialization::pimpl::Data>(keyData);
                    auto cached = nearCache->get(key);
                    if (cached) {
                        if (internal::nearcache::NearCache<K, V>::NULL_OBJECT == cached) {
                            return boost::make_ready_future(std::unique_ptr<serialization::pimpl::Data>());
                        }
                        return boost::make_ready_future(std::unique_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(*cached)));
                    }

                    bool marked = keyStateMarker->tryMark(*key);

                    try {
                        auto future = IMap::getInternal(*key);
                        if (marked) {
                            return future.then(boost::launch::deferred, [=](boost::future<std::unique_ptr<serialization::pimpl::Data>> f) {
                                auto data = f.get();
                                auto cachedValue = data ? std::make_shared<serialization::pimpl::Data>(*data)
                                                        : internal::nearcache::NearCache<K, V>::NULL_OBJECT;
                                tryToPutNearCache(key, cachedValue);
                                return data;
                            });
                        }
                        return future;
                    } catch (exception::IException &) {
                        resetToUnmarkedState(key);
                        throw;
                    }
                }

                boost::future<bool> removeInternal(
                        const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) override {
                    try {
                        auto response = IMap::removeInternal(key, value);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>> removeInternal(
                        const serialization::pimpl::Data &keyData) override {
                    try {
                        invalidateNearCache(keyData);
                        auto response = IMap::removeInternal(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                removeAllInternal(const serialization::pimpl::Data &predicateData) override {
                    try {
                        auto response = IMap::removeAllInternal(predicateData);
                        nearCache->clear();
                        return response;
                    } catch (exception::IException &) {
                        nearCache->clear();
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage> deleteInternal(const serialization::pimpl::Data &key) override {
                    try {
                        auto response = IMap::deleteInternal(key);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                boost::future<bool> tryRemoveInternal(const serialization::pimpl::Data &keyData,
                                                              std::chrono::steady_clock::duration timeout) override {
                    try {
                        auto response = IMap::tryRemoveInternal(keyData, timeout);
                        invalidateNearCache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<bool> tryPutInternal(const serialization::pimpl::Data &keyData,
                        const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration timeout) override {
                    try {
                        auto response = IMap::tryPutInternal(keyData, valueData, timeout);
                        invalidateNearCache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>> putInternal(const serialization::pimpl::Data &keyData,
                        const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration ttl) override {
                    try {
                        auto previousValue = IMap::putInternal(keyData, valueData, ttl);
                        invalidateNearCache(keyData);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                        const serialization::pimpl::Data &valueData,
                                        std::chrono::steady_clock::duration ttl) override {
                    try {
                        auto result = IMap::tryPutTransientInternal(keyData, valueData, ttl);
                        invalidateNearCache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    std::chrono::steady_clock::duration ttl) override {
                    try {
                        auto previousValue = IMap::putIfAbsentData(keyData, valueData, ttl);
                        invalidateNearCache(keyData);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<bool> replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                          const serialization::pimpl::Data &valueData,
                                                          const serialization::pimpl::Data &newValueData) override {
                    try {
                        auto result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                        invalidateNearCache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                replaceInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) override {
                    try {
                        auto value = proxy::IMapImpl::replaceData(keyData, valueData);
                        invalidateNearCache(keyData);
                        return value;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            std::chrono::steady_clock::duration ttl) override {
                    try {
                        auto result = proxy::IMapImpl::set(keyData, valueData, ttl);
                        invalidateNearCache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<bool> evictInternal(const serialization::pimpl::Data &keyData) override {
                    try {
                        auto evicted = proxy::IMapImpl::evict(keyData);
                        invalidateNearCache(keyData);
                        return evicted;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<EntryVector>
                getAllInternal(int partitionId, const std::vector<serialization::pimpl::Data> &partitionKeys) override {
                    auto markers = std::make_shared<MARKER_MAP>();
                    try {
                        EntryVector result;
                        std::vector<serialization::pimpl::Data> remainingKeys;
                        for (auto &key : partitionKeys) {
                            auto keyData = std::make_shared<serialization::pimpl::Data>(key);
                            auto cached = nearCache->get(keyData);
                            if (cached && internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached) {
                                result.push_back(std::make_pair(std::move(*keyData), *cached));
                            } else {
                                if (invalidateOnChange) {
                                    (*markers)[keyData] = keyStateMarker->tryMark(*keyData);
                                }
                                remainingKeys.push_back(std::move(*keyData));
                            }
                        }

                        if (remainingKeys.empty()) {
                            return boost::make_ready_future(result);
                        }

                        return IMap::getAllInternal(partitionId, remainingKeys).then(
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
                                    tryToPutNearCache(key, value);
                                } else {
                                    nearCache->put(key, value);
                                }
                                allEntries.push_back(std::make_pair(std::move(*key), std::move(*value)));
                            }

                            unmarkRemainingMarkedKeys(*markers);
                            return allEntries;
                        });
                    } catch (exception::IException &) {
                        unmarkRemainingMarkedKeys(*markers);
                        throw;
                    }
                }

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &processor) override {
                    try {
                        auto response = IMap::executeOnKeyData(keyData, processor);
                        invalidateNearCache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                boost::future<protocol::ClientMessage>
                putAllInternal(int partitionId, const EntryVector &entries) override {
                    try {
                        auto result = IMap::putAllInternal(partitionId, entries);
                        invalidateEntries(entries);
                        return result;
                    } catch (exception::IException &) {
                        invalidateEntries(entries);
                        throw;
                    }
                }

                void invalidateEntries(const EntryVector &entries) {
                    for (auto &entry : entries) {
                        invalidateNearCache(std::make_shared<serialization::pimpl::Data>(entry.first));
                    }
                }

            private:
                impl::nearcache::KeyStateMarker *getKeyStateMarker() {
                    return std::static_pointer_cast<
                            impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, V> >(nearCache)->
                            getKeyStateMarker();
                }

                void addNearCacheInvalidateListener(std::unique_ptr<client::impl::BaseEventHandler> &handler) {
                    try {
                        invalidationListenerId = proxy::ProxyImpl::registerListener(createNearCacheEntryListenerCodec(),
                                                                                    std::move(handler)).get();
                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "-----------------\n Near Cache is not initialized!!! \n-----------------";
                        out << e.what();
                        logger.severe(out.str());
                    }
                }

                void removeNearCacheInvalidationListener() {
                    std::string listenerId = this->invalidationListenerId;
                    if (listenerId.empty()) {
                        return;
                    }

                    proxy::ProxyImpl::deregisterListener(listenerId).get();
                }

                class ClientMapAddNearCacheEventHandler
                        : public protocol::codec::MapAddNearCacheEntryListenerCodec::AbstractEventHandler {
                public:
                    ClientMapAddNearCacheEventHandler(
                            const std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > &cache)
                            : nearCache(cache) {
                    }

                    //@Override
                    void beforeListenerRegister() {
                        nearCache->clear();
                    }

                    //@Override
                    void onListenerRegister() {
                        nearCache->clear();
                    }

                    //@Override
                    virtual void handleIMapInvalidationEventV10(std::unique_ptr<Data> &key) {
                        // null key means Near Cache has to remove all entries in it (see MapAddNearCacheEntryListenerMessageTask)
                        if (key.get() == NULL) {
                            nearCache->clear();
                        } else {
                            nearCache->invalidate(std::shared_ptr<serialization::pimpl::Data>(std::move(key)));
                        }
                    }

                    //@Override
                    virtual void handleIMapBatchInvalidationEventV10(const std::vector<Data> &keys) {
                        for (std::vector<serialization::pimpl::Data>::const_iterator it = keys.begin();
                             it != keys.end(); ++it) {
                            nearCache->invalidate(std::shared_ptr<serialization::pimpl::Data>(
                                    new serialization::pimpl::Data(*it)));
                        }
                    }

                    //@Override
                    virtual void handleIMapInvalidationEventV14(std::unique_ptr<serialization::pimpl::Data> &key,
                                                                const std::string &sourceUuid,
                                                                const util::UUID &partitionUuid,
                                                                const int64_t &sequence) {
                        // TODO: change with the new near cache impl.
                        handleIMapInvalidationEventV10(key);
                    }

                    //@Override
                    virtual void
                    handleIMapBatchInvalidationEventV14(const std::vector<serialization::pimpl::Data> &keys,
                                                        const std::vector<std::string> &sourceUuids,
                                                        const std::vector<util::UUID> &partitionUuids,
                                                        const std::vector<int64_t> &sequences) {
                        // TODO: change with the new near cache impl.
                        handleIMapBatchInvalidationEventV10(keys);
                    }

                private:
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache;
                };

                class NearCacheEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::MapAddNearCacheEntryListenerCodec::encodeRequest(name,
                                                                                                 static_cast<int32_t>(listenerFlags),
                                                                                                 localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                    NearCacheEntryListenerMessageCodec(const std::string &name, EntryEvent::type listenerFlags)
                            : name(name), listenerFlags(listenerFlags) {}

                private:
                    const std::string &name;
                    EntryEvent::type listenerFlags;
                };

                std::unique_ptr<spi::impl::ListenerMessageCodec> createNearCacheEntryListenerCodec() {
                    EntryEvent::type listenerFlags = EntryEvent::type::INVALIDATION;
                    return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheEntryListenerMessageCodec(spi::ClientProxy::getName(), listenerFlags));
                }

                void resetToUnmarkedState(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    if (keyStateMarker->tryUnmark(*key)) {
                        return;
                    }

                    invalidateNearCache(key);
                    keyStateMarker->forceUnmark(*key);
                }

                void unmarkRemainingMarkedKeys(MARKER_MAP &markers) {
                    for (MARKER_MAP::const_iterator it = markers.begin();
                         it != markers.end(); ++it) {
                        if (it->second) {
                            keyStateMarker->forceUnmark(*it->first);
                        }
                    }
                }

                void tryToPutNearCache(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                       const std::shared_ptr<V> &response) {
                    tryToPutNearCacheInternal<V>(keyData, response);
                }

/*
                void tryToPutNearCache(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                       const std::shared_ptr<serialization::pimpl::Data> &response) {
                    tryToPutNearCacheInternal<serialization::pimpl::Data>(keyData, response);
                }
*/

                template<typename VALUETYPE>
                void tryToPutNearCacheInternal(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                               const std::shared_ptr<VALUETYPE> &response) {
                    try {
                        nearCache->put(keyData, response);
                        resetToUnmarkedState(keyData);
                    } catch (exception::IException &) {
                        resetToUnmarkedState(keyData);
                        throw;
                    }
                }

                /**
                 * This method modifies the key Data internal pointer although it is marked as const
                 * @param key The key for which to invalidate the near cache
                 */
                void invalidateNearCache(const serialization::pimpl::Data &key) {
                    nearCache->invalidate(std::make_shared<serialization::pimpl::Data>(key));
                }

                void invalidateNearCache(std::shared_ptr<serialization::pimpl::Data> key) {
                    nearCache->invalidate(key);
                }

                bool cacheLocalEntries;
                bool invalidateOnChange;
                impl::nearcache::KeyStateMarker *keyStateMarker;
                const config::NearCacheConfig<K, V> &nearCacheConfig;
                std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V>> nearCache;
                std::string invalidationListenerId;
                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

