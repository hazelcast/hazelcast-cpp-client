/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/foreach.hpp>
#include "hazelcast/client/protocol/codec/MapRemoveEntryListenerCodec.h"

#include "hazelcast/client/mixedtype/NearCachedClientMapProxy.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            NearCachedClientMapProxy::NearCachedClientMapProxy(const std::string &instanceName,
                                                               spi::ClientContext *context,
                                                               const mixedtype::config::MixedNearCacheConfig &config)
                    : ClientMapProxy(instanceName, context), cacheLocalEntries(false), invalidateOnChange(false),
                      keyStateMarker(NULL), nearCacheConfig(config) {
            }

            monitor::LocalMapStats &NearCachedClientMapProxy::getLocalMapStats() {
                monitor::LocalMapStats &localMapStats = ClientMapProxy::getLocalMapStats();
                monitor::NearCacheStats &nearCacheStats = nearCache->getNearCacheStats();
                ((monitor::impl::LocalMapStatsImpl &) localMapStats).setNearCacheStats(nearCacheStats);
                return localMapStats;
            }

            void NearCachedClientMapProxy::onInitialize() {
                ClientMapProxy::onInitialize();

                internal::nearcache::NearCacheManager &nearCacheManager = getContext().getNearCacheManager();
                cacheLocalEntries = nearCacheConfig.isCacheLocalEntries();
                int partitionCount = getContext().getPartitionService().getPartitionCount();
                nearCache = nearCacheManager.getOrCreateNearCache<TypedData, TypedData, serialization::pimpl::Data>(
                        proxy::ProxyImpl::getName(), nearCacheConfig);

                nearCache = map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData>::asInvalidationAware(
                        nearCache, partitionCount);

                keyStateMarker = getKeyStateMarker();

                invalidateOnChange = nearCache->isInvalidatedOnChange();
                if (invalidateOnChange) {
                    std::auto_ptr<client::impl::BaseEventHandler> invalidationHandler(
                            new ClientMapAddNearCacheEventHandler(nearCache));
                    addNearCacheInvalidateListener(invalidationHandler);
                }
            }

            //@Override
            bool NearCachedClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                boost::shared_ptr<serialization::pimpl::Data> key = toShared(keyData);
                boost::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    return internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT != cached;
                }

                return ClientMapProxy::containsKeyInternal(*key);
            }

            //@override
            boost::shared_ptr<TypedData> NearCachedClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                boost::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(keyData);
                boost::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    if (internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT == cached) {
                        return boost::shared_ptr<TypedData>(
                                new TypedData(std::auto_ptr<serialization::pimpl::Data>(), getSerializationService()));
                    }
                    return cached;
                }

                bool marked = keyStateMarker->tryMark(*key);

                try {
                    boost::shared_ptr<TypedData> value = ClientMapProxy::getInternal(*key);
                    if (marked && value->getData().get()) {
                        tryToPutNearCache(key, value);
                    }
                    return value;
                } catch (exception::IException &) {
                    resetToUnmarkedState(key);
                    throw;
                }
            }

            //@Override
            std::auto_ptr<serialization::pimpl::Data> NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key) {
                std::auto_ptr<serialization::pimpl::Data> responseData = ClientMapProxy::removeInternal(key);
                invalidateNearCache(key);
                return responseData;
            }

            //@Override
            bool NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                bool response = ClientMapProxy::removeInternal(key, value);
                invalidateNearCache(key);
                return response;
            }

            void NearCachedClientMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                try {
                    ClientMapProxy::removeAllInternal(predicateData);

                    nearCache->clear();
                } catch (exception::IException &) {
                    nearCache->clear();
                    throw;
                }
            }

            void NearCachedClientMapProxy::deleteInternal(const serialization::pimpl::Data &key) {
                ClientMapProxy::deleteInternal(key);
                invalidateNearCache(key);
            }

            bool
            NearCachedClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis) {
                bool response = ClientMapProxy::tryRemoveInternal(key, timeoutInMillis);
                invalidateNearCache(key);
                return response;
            }

            bool NearCachedClientMapProxy::tryPutInternal(const serialization::pimpl::Data &key,
                                                          const serialization::pimpl::Data &value,
                                                          long timeoutInMillis) {
                bool response = ClientMapProxy::tryPutInternal(key, value, timeoutInMillis);
                invalidateNearCache(key);
                return response;
            }

            std::auto_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putInternal(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value,
                                                  long timeoutInMillis) {
                std::auto_ptr<serialization::pimpl::Data> previousValue =
                        ClientMapProxy::putInternal(key, value, timeoutInMillis);
                invalidateNearCache(key);
                return previousValue;
            }

            void NearCachedClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &key,
                                                                   const serialization::pimpl::Data &value,
                                                                   int ttlInMillis) {
                ClientMapProxy::tryPutTransientInternal(key, value, ttlInMillis);
                invalidateNearCache(key);
            }

            std::auto_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                          const serialization::pimpl::Data &valueData,
                                                          int ttlInMillis) {
                std::auto_ptr<serialization::pimpl::Data> previousValue =
                        ClientMapProxy::putIfAbsentData(keyData, valueData, ttlInMillis);
                invalidateNearCache(keyData);
                return previousValue;
            }

            bool NearCachedClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                                 const serialization::pimpl::Data &valueData,
                                                                 const serialization::pimpl::Data &newValueData) {
                bool result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
                invalidateNearCache(keyData);
                return result;
            }

            std::auto_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                                      const serialization::pimpl::Data &valueData) {
                std::auto_ptr<serialization::pimpl::Data> value =
                        proxy::IMapImpl::replaceData(keyData, valueData);
                invalidateNearCache(keyData);
                return value;
            }

            void NearCachedClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       int ttlInMillis) {
                proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                invalidateNearCache(keyData);
            }

            bool NearCachedClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                bool evicted = proxy::IMapImpl::evict(keyData);
                invalidateNearCache(keyData);
                return evicted;
            }

            EntryVector NearCachedClientMapProxy::getAllInternal(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData) {
                MARKER_MAP markers;
                try {
                    ClientMapProxy::PID_TO_KEY_MAP nonCachedPidToKeyMap;
                    EntryVector result = populateFromNearCache(pIdToKeyData, nonCachedPidToKeyMap, markers);

                    EntryVector responses = ClientMapProxy::getAllInternal(nonCachedPidToKeyMap);
                    BOOST_FOREACH(const EntryVector::value_type &entry, responses) {
                                    boost::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(
                                            entry.first);
                                    boost::shared_ptr<TypedData> value = boost::shared_ptr<TypedData>(new TypedData(
                                            std::auto_ptr<serialization::pimpl::Data>(
                                                    new serialization::pimpl::Data(entry.second)),
                                            getSerializationService()));
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

                    result.insert(result.end(), responses.begin(), responses.end());

                    return result;
                } catch (exception::IException &) {
                    unmarkRemainingMarkedKeys(markers);
                    throw;
                }
            }

            std::auto_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                           const serialization::pimpl::Data &processor) {
                std::auto_ptr<serialization::pimpl::Data> response =
                        ClientMapProxy::executeOnKeyData(keyData, processor);
                invalidateNearCache(keyData);
                return response;
            }

            void
            NearCachedClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                ClientMapProxy::putAllInternal(entries);

                for (std::map<int, EntryVector>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                    for (EntryVector::const_iterator entryIt = it->second.begin();
                         entryIt != it->second.end(); ++entryIt) {
                        invalidateNearCache(ClientMapProxy::toShared(entryIt->first));
                    }
                }
            }

            map::impl::nearcache::KeyStateMarker *NearCachedClientMapProxy::getKeyStateMarker() {
                return boost::static_pointer_cast<
                        map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData> >(
                        nearCache)->
                        getKeyStateMarker();
            }

            void NearCachedClientMapProxy::addNearCacheInvalidateListener(
                    std::auto_ptr<client::impl::BaseEventHandler> handler) {
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

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            NearCachedClientMapProxy::createNearCacheEntryListenerCodec() {
                int32_t listenerFlags = EntryEventType::INVALIDATION;
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new NearCacheEntryListenerMessageCodec(getName(), listenerFlags));
            }

            void NearCachedClientMapProxy::resetToUnmarkedState(boost::shared_ptr<serialization::pimpl::Data> &key) {
                if (keyStateMarker->tryUnmark(*key)) {
                    return;
                }

                invalidateNearCache(key);
                keyStateMarker->forceUnmark(*key);
            }

            void NearCachedClientMapProxy::unmarkRemainingMarkedKeys(
                    std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> &markers) {
                for (std::map<boost::shared_ptr<serialization::pimpl::Data>, bool>::const_iterator it = markers.begin();
                     it != markers.end(); ++it) {
                    if (it->second) {
                        keyStateMarker->forceUnmark(*it->first);
                    }
                }
            }

            void NearCachedClientMapProxy::tryToPutNearCache(boost::shared_ptr<serialization::pimpl::Data> &keyData,
                                                             boost::shared_ptr<TypedData> &response) {
                try {
                    if (response.get()) {
                        nearCache->put(keyData, response);
                    }
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
            void NearCachedClientMapProxy::invalidateNearCache(const serialization::pimpl::Data &key) {
                nearCache->remove(ClientMapProxy::toShared(key));
            }

            void NearCachedClientMapProxy::invalidateNearCache(boost::shared_ptr<serialization::pimpl::Data> key) {
                nearCache->remove(key);
            }

            EntryVector
            NearCachedClientMapProxy::populateFromNearCache(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData,
                                                            PID_TO_KEY_MAP &nonCachedPidToKeyMap, MARKER_MAP &markers) {
                EntryVector result;

                BOOST_FOREACH(const ClientMapProxy::PID_TO_KEY_MAP::value_type &partitionDatas, pIdToKeyData) {
                                typedef std::vector<boost::shared_ptr<serialization::pimpl::Data> > SHARED_DATA_VECTOR;
                                SHARED_DATA_VECTOR nonCachedData;
                                BOOST_FOREACH(const SHARED_DATA_VECTOR::value_type &keyData, partitionDatas.second) {
                                                boost::shared_ptr<TypedData> cached = nearCache->get(keyData);
                                                if (cached.get() != NULL && !cached->getData().get() &&
                                                    internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT !=
                                                    cached) {
                                                    serialization::pimpl::Data valueData(*cached->getData());
                                                    result.push_back(std::make_pair(*keyData, valueData));
                                                } else if (invalidateOnChange) {
                                                    markers[keyData] = keyStateMarker->tryMark(*keyData);
                                                    nonCachedData.push_back(keyData);
                                                }
                                            }
                                nonCachedPidToKeyMap[partitionDatas.first] = nonCachedData;
                            }
                return result;
            }

            std::auto_ptr<protocol::ClientMessage>
            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::encodeRequest(name, listenerFlags,
                                                                                         localOnly);
            }

            std::string NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::auto_ptr<protocol::ClientMessage>
            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::NearCacheEntryListenerMessageCodec(
                    const std::string &name, int32_t listenerFlags) : name(name), listenerFlags(listenerFlags) {}
        }
    }
}
