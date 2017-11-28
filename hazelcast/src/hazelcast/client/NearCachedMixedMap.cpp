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

#include "hazelcast/client/NearCachedMixedMap.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/adapter/IMapDataStructureAdapter.h"
#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/EntryEvent.h"

namespace hazelcast {
    namespace client {
        NearCachedMixedMap::NearCachedMixedMap(const std::string &instanceName, spi::ClientContext *context,
                                                       const hazelcast::client::config::MixedNearCacheConfig &config)
                : MixedMap(instanceName, context), cacheLocalEntries(false), invalidateOnChange(false),
                  keyStateMarker(NULL), nearCacheConfig(config) {
        }

        monitor::LocalMapStats &NearCachedMixedMap::getLocalMapStats() {
            monitor::LocalMapStats &localMapStats = MixedMap::getLocalMapStats();
            monitor::NearCacheStats &nearCacheStats = nearCache->getNearCacheStats();
            ((monitor::impl::LocalMapStatsImpl &) localMapStats).setNearCacheStats(nearCacheStats);
            return localMapStats;
        }

        void NearCachedMixedMap::onInitialize() {
            MixedMap::onInitialize();

            internal::nearcache::NearCacheManager &nearCacheManager = context->getNearCacheManager();
            cacheLocalEntries = nearCacheConfig.isCacheLocalEntries();
            std::auto_ptr<internal::adapter::DataStructureAdapter<TypedData, TypedData> > adapter;/*(
                    new internal::adapter::IMapDataStructureAdapter<TypedData, TypedData>(*this));*/
            int partitionCount = context->getPartitionService().getPartitionCount();
            nearCache = nearCacheManager.getOrCreateNearCache<TypedData, TypedData, serialization::pimpl::Data>(
                    proxy::ProxyImpl::getName(), nearCacheConfig, adapter);

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
        bool NearCachedMixedMap::containsKeyInternal(const serialization::pimpl::Data &keyData) {
            boost::shared_ptr<serialization::pimpl::Data> key = toShared(keyData);
            boost::shared_ptr<TypedData> cached = nearCache->get(key);
            if (cached.get() != NULL) {
                return internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT != cached;
            }

            return MixedMap::containsKeyInternal(*key);
        }

        //@override
        boost::shared_ptr<TypedData> NearCachedMixedMap::getInternal(serialization::pimpl::Data &keyData) {
            boost::shared_ptr<serialization::pimpl::Data> key = MixedMap::toShared(keyData);
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
                boost::shared_ptr<TypedData> value = MixedMap::getInternal(*key);
                if (marked && value->getData()) {
                    tryToPutNearCache(key, value);
                }
                return value;
            } catch (exception::IException &) {
                resetToUnmarkedState(key);
                throw;
            }
        }

        //@Override
        std::auto_ptr<serialization::pimpl::Data> NearCachedMixedMap::removeInternal(
                const serialization::pimpl::Data &key) {
            std::auto_ptr<serialization::pimpl::Data> responseData = MixedMap::removeInternal(key);
            invalidateNearCache(key);
            return responseData;
        }

        //@Override
        bool NearCachedMixedMap::removeInternal(
                const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
            bool response = MixedMap::removeInternal(key, value);
            invalidateNearCache(key);
            return response;
        }

        void NearCachedMixedMap::removeAllInternal(const serialization::pimpl::Data &predicateData) {
            try {
                MixedMap::removeAllInternal(predicateData);

                nearCache->clear();
            } catch (exception::IException &) {
                nearCache->clear();
                throw;
            }
        }

        void NearCachedMixedMap::deleteInternal(const serialization::pimpl::Data &key) {
            MixedMap::deleteInternal(key);
            invalidateNearCache(key);
        }

        bool NearCachedMixedMap::tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis) {
            bool response = MixedMap::tryRemoveInternal(key, timeoutInMillis);
            invalidateNearCache(key);
            return response;
        }

        bool NearCachedMixedMap::tryPutInternal(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                    long timeoutInMillis) {
            bool response = MixedMap::tryPutInternal(key, value, timeoutInMillis);
            invalidateNearCache(key);
            return response;
        }

        std::auto_ptr<serialization::pimpl::Data> NearCachedMixedMap::putInternal(const serialization::pimpl::Data &key,
                                                                      const serialization::pimpl::Data &value,
                                                                      long timeoutInMillis) {
            std::auto_ptr<serialization::pimpl::Data> previousValue =
                    MixedMap::putInternal(key, value, timeoutInMillis);
            invalidateNearCache(key);
            return previousValue;
        }

        void NearCachedMixedMap::tryPutTransientInternal(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value, int ttlInMillis) {
            MixedMap::tryPutTransientInternal(key, value, ttlInMillis);
            invalidateNearCache(key);
        }

        std::auto_ptr<serialization::pimpl::Data> NearCachedMixedMap::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                                              const serialization::pimpl::Data &valueData,
                                                                              int ttlInMillis) {
            std::auto_ptr<serialization::pimpl::Data> previousValue =
                    MixedMap::putIfAbsentData(keyData, valueData, ttlInMillis);
            invalidateNearCache(keyData);
            return previousValue;
        }

        bool NearCachedMixedMap::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                           const serialization::pimpl::Data &valueData,
                                           const serialization::pimpl::Data &newValueData) {
            bool result = proxy::IMapImpl::replace(keyData, valueData, newValueData);
            invalidateNearCache(keyData);
            return result;
        }

        std::auto_ptr<serialization::pimpl::Data> NearCachedMixedMap::replaceInternal(const serialization::pimpl::Data &keyData,
                                                                          const serialization::pimpl::Data &valueData) {
            std::auto_ptr<serialization::pimpl::Data> value =
                    proxy::IMapImpl::replaceData(keyData, valueData);
            invalidateNearCache(keyData);
            return value;
        }

        void NearCachedMixedMap::setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                                 int ttlInMillis) {
            proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
            invalidateNearCache(keyData);
        }

        bool NearCachedMixedMap::evictInternal(const serialization::pimpl::Data &keyData) {
            bool evicted = proxy::IMapImpl::evict(keyData);
            invalidateNearCache(keyData);
            return evicted;
        }

        EntryVector NearCachedMixedMap::getAllInternal(MixedMap::PID_TO_KEY_MAP &pIdToKeyData) {
            MARKER_MAP markers;
            try {
                EntryVector result = populateFromNearCache(pIdToKeyData, markers);

                EntryVector responses = MixedMap::getAllInternal(pIdToKeyData);
                BOOST_FOREACH(const EntryVector::value_type &entry , responses) {
                                boost::shared_ptr<serialization::pimpl::Data> key = MixedMap::toShared(
                                        entry.first.clone());
                                boost::shared_ptr<TypedData> value = boost::shared_ptr<TypedData>(new TypedData(
                                        std::auto_ptr<serialization::pimpl::Data>(
                                                new serialization::pimpl::Data(entry.second.clone())),
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
        NearCachedMixedMap::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                             const serialization::pimpl::Data &processor) {
            std::auto_ptr<serialization::pimpl::Data> response =
                    MixedMap::executeOnKeyData(keyData, processor);
            invalidateNearCache(keyData);
            return response;
        }

        void
        NearCachedMixedMap::putAllInternal(const std::map<int, EntryVector> &entries) {
            MixedMap::putAllInternal(entries);

            for (std::map<int, EntryVector>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                for (EntryVector::const_iterator entryIt = it->second.begin();
                     entryIt != it->second.end(); ++entryIt) {
                    invalidateNearCache(MixedMap::toShared(entryIt->first));
                }
            }
        }

        map::impl::nearcache::KeyStateMarker *NearCachedMixedMap::getKeyStateMarker() {
            return boost::static_pointer_cast<
                    map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData> >(nearCache)->
                    getKeyStateMarker();
        }

        void NearCachedMixedMap::addNearCacheInvalidateListener(std::auto_ptr<client::impl::BaseEventHandler> handler) {
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


        std::auto_ptr<protocol::codec::IAddListenerCodec> NearCachedMixedMap::createNearCacheEntryListenerCodec() {
            return std::auto_ptr<protocol::codec::IAddListenerCodec>(
                    new protocol::codec::MapAddNearCacheEntryListenerCodec(
                            nearCache->getName(), EntryEventType::INVALIDATION, false));
        }

        void NearCachedMixedMap::resetToUnmarkedState(boost::shared_ptr<serialization::pimpl::Data> &key) {
            if (keyStateMarker->tryUnmark(*key)) {
                return;
            }

            invalidateNearCache(key);
            keyStateMarker->forceUnmark(*key);
        }

        void NearCachedMixedMap::unmarkRemainingMarkedKeys(std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> &markers) {
            for (std::map<boost::shared_ptr<serialization::pimpl::Data>, bool>::const_iterator it = markers.begin();
                 it != markers.end(); ++it) {
                if (it->second) {
                    keyStateMarker->forceUnmark(*it->first);
                }
            }
        }

        void NearCachedMixedMap::tryToPutNearCache(boost::shared_ptr<serialization::pimpl::Data> &keyData,
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
        void NearCachedMixedMap::invalidateNearCache(const serialization::pimpl::Data &key) {
            nearCache->remove(MixedMap::toShared(key));
        }

        void NearCachedMixedMap::invalidateNearCache(boost::shared_ptr<serialization::pimpl::Data> key) {
            nearCache->remove(key);
        }
        
        EntryVector
        NearCachedMixedMap::populateFromNearCache(MixedMap::PID_TO_KEY_MAP &pIdToKeyData, MARKER_MAP &markers) {
            EntryVector result;

            BOOST_FOREACH(MixedMap::PID_TO_KEY_MAP::value_type &partitionDatas , pIdToKeyData) {
                            typedef std::vector<boost::shared_ptr<serialization::pimpl::Data> > SHARED_DATA_VECTOR;
                            SHARED_DATA_VECTOR nonCachedData;
                            BOOST_FOREACH(const SHARED_DATA_VECTOR::value_type &keyData , partitionDatas.second) {
                                            boost::shared_ptr<TypedData> cached = nearCache->get(keyData);
                                            if (cached.get() != NULL && !cached->getData() &&
                                                internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT != cached) {
                                                serialization::pimpl::Data valueData(std::auto_ptr<std::vector<byte> >(
                                                        new std::vector<byte>(cached->getData()->toByteArray())));
                                                result.push_back(std::make_pair(*keyData, valueData));
                                            } else if (invalidateOnChange) {
                                                markers[keyData] = keyStateMarker->tryMark(*keyData);
                                                nonCachedData.push_back(keyData);
                                            }
                                        }
                            partitionDatas.second = nonCachedData;
                        }
            return result;
        }

    }
}
