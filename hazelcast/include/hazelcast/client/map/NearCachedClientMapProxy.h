/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_
#define HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/map/ClientMapProxy.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/executor/CompletedFuture.h"
#include "hazelcast/client/protocol/codec/MapRemoveEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"
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
            template<typename K, typename V>
            class NearCachedClientMapProxy
                    : public ClientMapProxy<K, V>,
                      public boost::enable_shared_from_this<NearCachedClientMapProxy<K, V> > {
            public:
                NearCachedClientMapProxy(const std::string &instanceName, spi::ClientContext *context,
                                         const config::NearCacheConfig<K, V> &config)
                        : ClientMapProxy<K, V>(instanceName, context), cacheLocalEntries(false),
                          invalidateOnChange(false), keyStateMarker(NULL), nearCacheConfig(config),
                          logger(context->getLogger()) {
                }

                virtual boost::shared_ptr<ICompletableFuture<V> > getAsync(const K &key) {
                    boost::shared_ptr<serialization::pimpl::Data> ncKey = ClientMapProxy<K, V>::toSharedData(key);
                    boost::shared_ptr<V> cached = nearCache->get(ncKey);
                    if (cached.get() != NULL) {
                        boost::shared_ptr<ExecutorService> executor = spi::ClientProxy::getContext().getClientExecutionService().getUserExecutor();
                        if (internal::nearcache::NearCache<K, V>::NULL_OBJECT == cached) {
                            return boost::shared_ptr<ICompletableFuture<V> >(
                                    new internal::executor::CompletedFuture<V>(boost::shared_ptr<V>(), executor));
                        }
                        return boost::shared_ptr<ICompletableFuture<V> >(
                                new internal::executor::CompletedFuture<V>(cached, executor));
                    }

                    bool marked = keyStateMarker->tryMark(*ncKey);
                    try {
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = ClientMapProxy<K, V>::getAsyncInternal(
                                *ncKey);
                        if (marked) {
                            boost::shared_ptr<ExecutionCallback<protocol::ClientMessage> > callback(
                                    new GetAsyncExecutionCallback(ncKey,
                                                                  boost::enable_shared_from_this<NearCachedClientMapProxy<K, V> >::shared_from_this()));
                            invocationFuture->andThen(callback,
                                                      spi::ClientProxy::getContext().getClientExecutionService().shared_from_this());
                        }
                        return boost::shared_ptr<ICompletableFuture<V> >(
                                new internal::ClientDelegatingFuture<V>(invocationFuture,
                                                                        ClientMapProxy<K, V>::getSerializationService(),
                                                                        ClientMapProxy<K, V>::PUT_ASYNC_RESPONSE_DECODER()));
                    } catch (exception::IException &e) {
                        resetToUnmarkedState(ncKey);
                        util::ExceptionUtil::rethrow(e);
                    }

                    return boost::shared_ptr<ICompletableFuture<V> >();
                }

                virtual monitor::LocalMapStats &getLocalMapStats() {
                    monitor::LocalMapStats &localMapStats = ClientMapProxy<K, V>::getLocalMapStats();
                    monitor::NearCacheStats &nearCacheStats = nearCache->getNearCacheStats();
                    ((monitor::impl::LocalMapStatsImpl &) localMapStats).setNearCacheStats(nearCacheStats);
                    return localMapStats;
                }

            protected:
                typedef std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> MARKER_MAP;

                //@override
                void onInitialize() {
                    ClientMapProxy<K, V>::onInitialize();

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
                        std::auto_ptr<client::impl::BaseEventHandler> invalidationHandler(
                                new ClientMapAddNearCacheEventHandler(nearCache));
                        addNearCacheInvalidateListener(invalidationHandler);
                    }
                }

                virtual void postDestroy() {
                    try {
                        removeNearCacheInvalidationListener();
                        spi::ClientProxy::getContext().getNearCacheManager().destroyNearCache(
                                spi::ClientProxy::getName());
                        spi::ClientProxy::postDestroy();
                    } catch (exception::IException &) {
                        spi::ClientProxy::postDestroy();
                    }
                }

                virtual void onDestroy() {
                    removeNearCacheInvalidationListener();
                    spi::ClientProxy::getContext().getNearCacheManager().destroyNearCache(spi::ClientProxy::getName());

                    ClientMapProxy<K, V>::onShutdown();
                }

                //@Override
                bool containsKeyInternal(const serialization::pimpl::Data &keyData) {
                    boost::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy<K, V>::toShared(keyData);
                    boost::shared_ptr<V> cached = nearCache->get(key);
                    if (cached.get() != NULL) {
                        return internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached;
                    }

                    return ClientMapProxy<K, V>::containsKeyInternal(*key);
                }

                //@override
                boost::shared_ptr<V> getInternal(serialization::pimpl::Data &keyData) {
                    boost::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy<K, V>::toShared(keyData);
                    boost::shared_ptr<V> cached = nearCache->get(key);
                    if (cached.get() != NULL) {
                        if (internal::nearcache::NearCache<K, V>::NULL_OBJECT == cached) {
                            return boost::shared_ptr<V>();
                        }
                        return cached;
                    }

                    bool marked = keyStateMarker->tryMark(*key);

                    try {
                        boost::shared_ptr<V> value = ClientMapProxy<K, V>::getInternal(*key);
                        if (marked) {
                            tryToPutNearCache(key, value);
                        }
                        return value;
                    } catch (exception::IException &) {
                        resetToUnmarkedState(key);
                        throw;
                    }
                }

                //@Override
                virtual bool removeInternal(
                        const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                    try {
                        bool response = ClientMapProxy<K, V>::removeInternal(key, value);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                //@Override
                virtual std::auto_ptr<serialization::pimpl::Data> removeInternal(
                        const serialization::pimpl::Data &key) {
                    try {
                        std::auto_ptr<serialization::pimpl::Data> response = ClientMapProxy<K, V>::removeInternal(key);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual boost::shared_ptr<ICompletableFuture<V> >
                removeAsyncInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        boost::shared_ptr<ICompletableFuture<V> > future = ClientMapProxy<K, V>::removeAsyncInternal(
                                keyData);
                        invalidateNearCache(keyData);
                        return future;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual void removeAllInternal(const serialization::pimpl::Data &predicateData) {
                    try {
                        ClientMapProxy<K, V>::removeAllInternal(predicateData);

                        nearCache->clear();
                    } catch (exception::IException &) {
                        nearCache->clear();
                        throw;
                    }
                }

                virtual void deleteInternal(const serialization::pimpl::Data &key) {
                    try {
                        ClientMapProxy<K, V>::deleteInternal(key);
                        invalidateNearCache(key);
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual bool tryRemoveInternal(const serialization::pimpl::Data &key, int64_t timeoutInMillis) {
                    try {
                        bool response = ClientMapProxy<K, V>::tryRemoveInternal(key, timeoutInMillis);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual bool
                tryPutInternal(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                               int64_t timeoutInMillis) {
                    try {
                        bool response = ClientMapProxy<K, V>::tryPutInternal(key, value, timeoutInMillis);
                        invalidateNearCache(key);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual std::auto_ptr<serialization::pimpl::Data> putInternal(const serialization::pimpl::Data &key,
                                                                              const serialization::pimpl::Data &value,
                                                                              int64_t timeoutInMillis) {
                    try {
                        std::auto_ptr<serialization::pimpl::Data> previousValue =
                                ClientMapProxy<K, V>::putInternal(key, value, timeoutInMillis);
                        invalidateNearCache(key);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual boost::shared_ptr<ICompletableFuture<V> >
                putAsyncInternal(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData, const V &value) {
                    try {
                        boost::shared_ptr<ICompletableFuture<V> > future = ClientMapProxy<K, V>::putAsyncInternal(ttl,
                                                                                                                  ttlUnit,
                                                                                                                  maxIdle,
                                                                                                                  maxIdleUnit,
                                                                                                                  keyData,
                                                                                                                  value);
                        invalidateNearCache(keyData);
                        return future;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual void tryPutTransientInternal(const serialization::pimpl::Data &key,
                                                     const serialization::pimpl::Data &value, int64_t ttlInMillis) {
                    try {
                        ClientMapProxy<K, V>::tryPutTransientInternal(key, value, ttlInMillis);
                        invalidateNearCache(key);
                    } catch (exception::IException &) {
                        invalidateNearCache(key);
                        throw;
                    }
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    int64_t ttlInMillis) {
                    try {
                        std::auto_ptr<serialization::pimpl::Data> previousValue =
                                ClientMapProxy<K, V>::putIfAbsentData(keyData, valueData, ttlInMillis);
                        invalidateNearCache(keyData);
                        return previousValue;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual bool replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                   const serialization::pimpl::Data &valueData,
                                                   const serialization::pimpl::Data &newValueData) {
                    try {
                        bool result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                        invalidateNearCache(keyData);
                        return result;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                replaceInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) {
                    std::auto_ptr<serialization::pimpl::Data> value =
                            proxy::IMapImpl::replaceData(keyData, valueData);
                    invalidateNearCache(keyData);
                    return value;
                }

                virtual void
                setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            int64_t ttlInMillis) {
                    try {
                        proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                        invalidateNearCache(keyData);
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual boost::shared_ptr<ICompletableFuture<void> >
                setAsyncInternal(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData, const V &value) {
                    try {
                        boost::shared_ptr<ICompletableFuture<void> > future = ClientMapProxy<K, V>::setAsyncInternal(
                                ttl,
                                ttlUnit,
                                maxIdle,
                                maxIdleUnit,
                                keyData,
                                value);
                        invalidateNearCache(keyData);
                        return future;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual bool evictInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        bool evicted = proxy::IMapImpl::evict(keyData);
                        invalidateNearCache(keyData);
                        return evicted;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual EntryVector
                getAllInternal(
                        const std::map<int, std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR> > &pIdToKeyData,
                        std::map<K, V> &result) {
                    MARKER_MAP markers;
                    try {
                        for (typename std::map<int, std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR> >::const_iterator
                                     it = pIdToKeyData.begin(); it != pIdToKeyData.end(); ++it) {
                            for (typename std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR>::const_iterator valueIterator = it->second.begin();
                                 valueIterator != it->second.end(); ++valueIterator) {
                                const boost::shared_ptr<serialization::pimpl::Data> &keyData = (*valueIterator).second;
                                boost::shared_ptr<V> cached = nearCache->get(keyData);
                                if (cached.get() != NULL &&
                                    internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached) {
                                    // Use insert method instead of '[]' operator to prevent the need for
                                    // std::is_default_constructible requirement for key and value
                                    result.insert(std::make_pair(*proxy::ProxyImpl::toObject<K>(*keyData), *cached));
                                } else if (invalidateOnChange) {
                                    markers[keyData] = keyStateMarker->tryMark(*keyData);
                                }
                            }
                        }

                        EntryVector responses = ClientMapProxy<K, V>::getAllInternal(pIdToKeyData, result);
                        for (EntryVector::const_iterator it = responses.begin(); it != responses.end(); ++it) {
                            boost::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy<K, V>::toShared(
                                    it->first);
                            boost::shared_ptr<serialization::pimpl::Data> value = ClientMapProxy<K, V>::toShared(
                                    it->second);
                            bool marked = false;
                            if (markers.count(key)) {
                                marked = markers[key];
                                markers.erase(key);
                            }

                            if (marked) {
                                tryToPutNearCache(key, value);
                            } else {
                                nearCache->put(key, value);
                            }
                        }

                        unmarkRemainingMarkedKeys(markers);

                        return responses;
                    } catch (exception::IException &) {
                        unmarkRemainingMarkedKeys(markers);
                        throw;
                    }
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &processor) {
                    try {
                        std::auto_ptr<serialization::pimpl::Data> response =
                                ClientMapProxy<K, V>::executeOnKeyData(keyData, processor);
                        invalidateNearCache(keyData);
                        return response;
                    } catch (exception::IException &) {
                        invalidateNearCache(keyData);
                        throw;
                    }
                }

                virtual void
                putAllInternal(const std::map<int, EntryVector> &entries) {
                    try {
                        ClientMapProxy<K, V>::putAllInternal(entries);
                        invalidateEntries(entries);
                    } catch (exception::IException &) {
                        invalidateEntries(entries);
                        throw;
                    }
                }

                void invalidateEntries(const std::map<int, EntryVector> &entries) {
                    for (std::map<int, EntryVector>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                        for (EntryVector::const_iterator entryIt = it->second.begin();
                             entryIt != it->second.end(); ++entryIt) {
                            invalidateNearCache(ClientMapProxy<K, V>::toShared(entryIt->first));
                        }
                    }
                }

            private:
                impl::nearcache::KeyStateMarker *getKeyStateMarker() {
                    return boost::static_pointer_cast<
                            impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, V> >(nearCache)->
                            getKeyStateMarker();
                }

                void addNearCacheInvalidateListener(std::auto_ptr<client::impl::BaseEventHandler> handler) {
                    try {
                        invalidationListenerId = proxy::ProxyImpl::registerListener(createNearCacheEntryListenerCodec(),
                                                                                    handler.release());
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

                    proxy::ProxyImpl::deregisterListener(listenerId);
                }


                class GetAsyncExecutionCallback : public ExecutionCallback<protocol::ClientMessage> {
                public:
                    GetAsyncExecutionCallback(const boost::shared_ptr<serialization::pimpl::Data> &ncKey,
                                              const boost::shared_ptr<NearCachedClientMapProxy<K, V> > &proxy) : ncKey(
                            ncKey), proxy(proxy) {}

                    virtual void onResponse(const boost::shared_ptr<protocol::ClientMessage> &response) {
                        boost::shared_ptr<V> value = ClientMapProxy<K, V>::GET_ASYNC_RESPONSE_DECODER()->decodeClientMessage(
                                response, proxy->getSerializationService());
                        proxy->tryToPutNearCache(ncKey, value);
                    }

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                        proxy->resetToUnmarkedState(ncKey);
                    }

                private:
                    boost::shared_ptr<serialization::pimpl::Data> ncKey;
                    boost::shared_ptr<NearCachedClientMapProxy<K, V> > proxy;

                };

                class ClientMapAddNearCacheEventHandler
                        : public protocol::codec::MapAddNearCacheEntryListenerCodec::AbstractEventHandler {
                public:
                    ClientMapAddNearCacheEventHandler(
                            const boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > &cache)
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
                    virtual void handleIMapInvalidationEventV10(std::auto_ptr<Data> key) {
                        // null key means Near Cache has to remove all entries in it (see MapAddNearCacheEntryListenerMessageTask)
                        if (key.get() == NULL) {
                            nearCache->clear();
                        } else {
                            nearCache->invalidate(boost::shared_ptr<serialization::pimpl::Data>(key));
                        }
                    }

                    //@Override
                    virtual void handleIMapBatchInvalidationEventV10(const std::vector<Data> &keys) {
                        for (std::vector<serialization::pimpl::Data>::const_iterator it = keys.begin();
                             it != keys.end(); ++it) {
                            nearCache->invalidate(boost::shared_ptr<serialization::pimpl::Data>(
                                    new serialization::pimpl::Data(*it)));
                        }
                    }

                    //@Override
                    virtual void handleIMapInvalidationEventV14(std::auto_ptr<serialization::pimpl::Data> key,
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
                    boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache;
                };

                class NearCacheEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::MapAddNearCacheEntryListenerCodec::encodeRequest(name, listenerFlags,
                                                                                                 localOnly);
                    }

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                                responseMessage).response;
                    }

                    std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
                    }

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(
                                clientMessage).response;
                    }

                    NearCacheEntryListenerMessageCodec(const std::string &name, int32_t listenerFlags)
                            : name(name), listenerFlags(listenerFlags) {}

                private:
                    const std::string &name;
                    int32_t listenerFlags;
                };

                boost::shared_ptr<spi::impl::ListenerMessageCodec> createNearCacheEntryListenerCodec() {
                    int32_t listenerFlags = EntryEventType::INVALIDATION;
                    return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheEntryListenerMessageCodec(spi::ClientProxy::getName(), listenerFlags));
                }

                void resetToUnmarkedState(boost::shared_ptr<serialization::pimpl::Data> &key) {
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

                void tryToPutNearCache(boost::shared_ptr<serialization::pimpl::Data> &keyData,
                                       boost::shared_ptr<V> &response) {
                    tryToPutNearCacheInternal<V>(keyData, response);
                }

                void tryToPutNearCache(boost::shared_ptr<serialization::pimpl::Data> &keyData,
                                       boost::shared_ptr<serialization::pimpl::Data> &response) {
                    tryToPutNearCacheInternal<serialization::pimpl::Data>(keyData, response);
                }

                template<typename VALUETYPE>
                void tryToPutNearCacheInternal(boost::shared_ptr<serialization::pimpl::Data> &keyData,
                                               boost::shared_ptr<VALUETYPE> &response) {
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
                    nearCache->invalidate(ClientMapProxy<K, V>::toShared(key));
                }

                void invalidateNearCache(boost::shared_ptr<serialization::pimpl::Data> key) {
                    nearCache->invalidate(key);
                }

                bool cacheLocalEntries;
                bool invalidateOnChange;
                impl::nearcache::KeyStateMarker *keyStateMarker;
                const config::NearCacheConfig<K, V> &nearCacheConfig;
                boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache;
                // since we don't have atomic support in the project yet, using shared_ptr
                util::Atomic<std::string> invalidationListenerId;
                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_ */

