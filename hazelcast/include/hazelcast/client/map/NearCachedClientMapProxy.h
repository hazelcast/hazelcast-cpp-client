/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/internal/adapter/IMapDataStructureAdapter.h"
#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"

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
            class NearCachedClientMapProxy : public ClientMapProxy<K, V> {
            public:
                NearCachedClientMapProxy(const std::string &instanceName, spi::ClientContext *context,
                                         const config::NearCacheConfig<K, V> &config)
                        : ClientMapProxy<K, V>(instanceName, context), cacheLocalEntries(false),
                          invalidateOnChange(false), keyStateMarker(NULL), nearCacheConfig(config) {
                }

                virtual monitor::LocalMapStats &getLocalMapStats() {
                    monitor::LocalMapStats &localMapStats = ClientMapProxy<K, V>::getLocalMapStats();
                    monitor::NearCacheStats &nearCacheStats = nearCache->getNearCacheStats();
                    ((monitor::impl::LocalMapStatsImpl &) localMapStats).setNearCacheStats(nearCacheStats);
                    return localMapStats;
                }
            protected:
                //@override
                void onInitialize() {
                    ClientMapProxy<K, V>::onInitialize();

                    internal::nearcache::NearCacheManager &nearCacheManager = this->context->getNearCacheManager();
                    cacheLocalEntries = nearCacheConfig.isCacheLocalEntries();
                    std::auto_ptr<internal::adapter::DataStructureAdapter<K, V> > adapter(
                            new internal::adapter::IMapDataStructureAdapter<K, V>(*this));
                    int partitionCount = this->context->getPartitionService().getPartitionCount();
                    nearCache = nearCacheManager.getOrCreateNearCache<K, V, serialization::pimpl::Data>(
                            proxy::ProxyImpl::getName(), nearCacheConfig, adapter);

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
                virtual std::auto_ptr<serialization::pimpl::Data> removeInternal(
                        const serialization::pimpl::Data &key) {
                    std::auto_ptr<serialization::pimpl::Data> responseData = ClientMapProxy<K, V>::removeInternal(key);
                    invalidateNearCache(key);
                    return responseData;
                }

                //@Override
                virtual bool removeInternal(
                        const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                    bool response = ClientMapProxy<K, V>::removeInternal(key, value);
                    invalidateNearCache(key);
                    return response;
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
                    ClientMapProxy<K, V>::deleteInternal(key);
                    invalidateNearCache(key);
                }

                virtual bool tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis) {
                    bool response = ClientMapProxy<K, V>::tryRemoveInternal(key, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                }

                virtual bool tryPutInternal(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                    long timeoutInMillis) {
                    bool response = ClientMapProxy<K, V>::tryPutInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                }

                virtual std::auto_ptr<serialization::pimpl::Data> putInternal(const serialization::pimpl::Data &key,
                                                                      const serialization::pimpl::Data &value,
                                                                      long timeoutInMillis) {
                    std::auto_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy<K, V>::putInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return previousValue;
                }

                virtual void tryPutTransientInternal(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value, int ttlInMillis) {
                    ClientMapProxy<K, V>::tryPutTransientInternal(key, value, ttlInMillis);
                    invalidateNearCache(key);
                }

                virtual std::auto_ptr<serialization::pimpl::Data> putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                                              const serialization::pimpl::Data &valueData,
                                                                              int ttlInMillis) {
                    std::auto_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy<K, V>::putIfAbsentData(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                    return previousValue;
                }

                virtual bool replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &valueData,
                                           const serialization::pimpl::Data &newValueData) {
                    bool result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                    invalidateNearCache(keyData);
                    return result;
                }

                virtual std::auto_ptr<serialization::pimpl::Data> replaceInternal(const serialization::pimpl::Data &keyData,
                                                                          const serialization::pimpl::Data &valueData) {
                    std::auto_ptr<serialization::pimpl::Data> value =
                            proxy::IMapImpl::replaceData(keyData, valueData);
                    invalidateNearCache(keyData);
                    return value;
                }

                virtual void setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                                 int ttlInMillis) {
                    proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                }

                virtual bool evictInternal(const serialization::pimpl::Data &keyData) {
                    bool evicted = proxy::IMapImpl::evict(keyData);
                    invalidateNearCache(keyData);
                    return evicted;
                }

                virtual EntryVector
                getAllInternal(const std::map<int, std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR> > &pIdToKeyData,
                               std::map<K, V> &result) {
                    std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> markers;
                    try {
                        for (typename std::map<int, std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR> >::const_iterator
                                     it = pIdToKeyData.begin(); it != pIdToKeyData.end(); ++it) {
                            for (typename std::vector<typename ClientMapProxy<K, V>::KEY_DATA_PAIR>::const_iterator valueIterator = it->second.begin();
                                 valueIterator != it->second.end(); ++valueIterator) {
                                const boost::shared_ptr<serialization::pimpl::Data> &keyData = (*valueIterator).second;
                                boost::shared_ptr<V> cached = nearCache->get(keyData);
                                if (cached.get() != NULL &&
                                    internal::nearcache::NearCache<K, V>::NULL_OBJECT != cached) {
                                    result[*proxy::ProxyImpl::toObject<K>(*keyData)] = *cached;
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
                    std::auto_ptr<serialization::pimpl::Data> response =
                            ClientMapProxy<K, V>::executeOnKeyData(keyData, processor);
                    invalidateNearCache(keyData);
                    return response;
                }

                virtual void
                putAllInternal(const std::map<int, EntryVector> &entries) {
                    ClientMapProxy<K, V>::putAllInternal(entries);

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
                        invalidationListenerId = boost::shared_ptr<std::string>(
                                new std::string(proxy::ProxyImpl::registerListener(
                                        createNearCacheEntryListenerCodec(), handler.release())));

                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "-----------------\n Near Cache is not initialized!!! \n-----------------";
                        out << e.what();
                        util::ILogger::getLogger().severe(out.str());
                    }
                }

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
                    void handleIMapInvalidation(std::auto_ptr<serialization::pimpl::Data> key) {
                        // null key means Near Cache has to remove all entries in it (see MapAddNearCacheEntryListenerMessageTask)
                        if (key.get() == NULL) {
                            nearCache->clear();
                        } else {
                            nearCache->remove(boost::shared_ptr<serialization::pimpl::Data>(key));
                        }
                    }

                    //@Override
                    void handleIMapBatchInvalidation(const std::vector<serialization::pimpl::Data> &keys) {
                        for (std::vector<serialization::pimpl::Data>::const_iterator it = keys.begin();
                             it != keys.end(); ++it) {
                            nearCache->remove(boost::shared_ptr<serialization::pimpl::Data>(
                                    new serialization::pimpl::Data(*it)));
                        }
                    }

                private:
                    boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache;
                };

                std::auto_ptr<protocol::codec::IAddListenerCodec> createNearCacheEntryListenerCodec() {
                    return std::auto_ptr<protocol::codec::IAddListenerCodec>(
                            new protocol::codec::MapAddNearCacheEntryListenerCodec(
                                    nearCache->getName(), EntryEventType::INVALIDATION, false));
                }

                void resetToUnmarkedState(boost::shared_ptr<serialization::pimpl::Data> &key) {
                    if (keyStateMarker->tryUnmark(*key)) {
                        return;
                    }

                    invalidateNearCache(key);
                    keyStateMarker->forceUnmark(*key);
                }

                void unmarkRemainingMarkedKeys(std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> &markers) {
                    for (std::map<boost::shared_ptr<serialization::pimpl::Data>, bool>::const_iterator it = markers.begin();
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
                    nearCache->remove(ClientMapProxy<K, V>::toShared(key));
                }

                void invalidateNearCache(boost::shared_ptr<serialization::pimpl::Data> key) {
                    nearCache->remove(key);
                }

                bool cacheLocalEntries;
                bool invalidateOnChange;
                impl::nearcache::KeyStateMarker *keyStateMarker;
                const config::NearCacheConfig<K, V> &nearCacheConfig;
                boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > nearCache;
                // since we don't have atomic support in the project yet, using shared_ptr
                boost::shared_ptr<std::string> invalidationListenerId;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_ */

