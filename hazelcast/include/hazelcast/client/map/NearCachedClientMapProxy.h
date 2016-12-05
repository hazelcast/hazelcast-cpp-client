/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include <stdexcept>
#include <climits>
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/map/ClientMapProxy.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarker.h"
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
                                         const config::NearCacheConfig *nearcacheConfig)
                        : ClientMapProxy<K, V>(instanceName, context) {
                    // TODO: implement it
                }
            protected:
                //@override
                void onInitialize() {
                    ClientMapProxy<K, V>::onInitialize();

                    initNearCache();
                }

                //@override
                boost::shared_ptr<V> getInternal(serialization::pimpl::Data &key) {
                    boost::shared_ptr<V> cached = nearCache->get(key);
                    if (cached.get() != NULL) {
                        if (internal::nearcache::NearCache<
                                K, V, internal::adapter::IMapDataStructureAdapter<K, V> >::NULL_OBJECT == cached) {
                            return boost::shared_ptr<V>();
                        }
                        return cached;
                    }

                    bool marked = keyStateMarker->tryMark(key);

                    boost::shared_ptr<V> value = ClientMapProxy<K, V>::getInternal(key);

                    if (marked) {
                        tryToPutNearCache(key, value);
                    }

                    return value;
                }

            private:
                void initNearCache() {
                    internal::nearcache::NearCacheManager &nearCacheManager = this->context->getNearCacheManager();
                    boost::shared_ptr<internal::adapter::IMapDataStructureAdapter<K, V> > adapter(
                            new internal::adapter::IMapDataStructureAdapter<K, V>(*this));
                    int partitionCount = this->context->getPartitionService().getPartitionCount();
                    nearCache = nearCacheManager.getOrCreateNearCache<serialization::pimpl::Data, K, V, internal::adapter::IMapDataStructureAdapter<K, V> >(
                            proxy::ProxyImpl::getName(), *nearCacheConfig, adapter, partitionCount);

                    keyStateMarker = &(getKeyStateMarker());

                    invalidateOnChange = nearCache->isInvalidatedOnChange();
                    if (invalidateOnChange) {
                        addNearCacheInvalidateListener();
                    }

                }

                internal::nearcache::impl::KeyStateMarker<serialization::pimpl::Data> &getKeyStateMarker() {
                    return nearCache->getKeyStateMarker();
                }

                void addNearCacheInvalidateListener() {
                    invalidationHandler = boost::shared_ptr<client::impl::BaseEventHandler>(
                            new ClientMapAddNearCacheEventHandler(nearCache));
                    addNearCacheInvalidateListener(invalidationHandler);
                }

                void addNearCacheInvalidateListener(boost::shared_ptr<client::impl::BaseEventHandler> handler) {
                    try {
                        invalidationListenerId = boost::shared_ptr<std::string>(
                                new std::string(proxy::ProxyImpl::registerListener(
                                        createNearCacheEntryListenerCodec(), handler.get())));

                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "-----------------\n Near Cache is not initialized!!! \n-----------------";
                        out << e.what();
                        util::ILogger::getLogger().severe(out.str());
                    }
                }

                class ClientMapAddNearCacheEventHandler : public protocol::codec::MapAddNearCacheEntryListenerCodec::AbstractEventHandler {
                public:
                    ClientMapAddNearCacheEventHandler(boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V, internal::adapter::IMapDataStructureAdapter<K, V> > > cache)
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
                            nearCache->remove(*key);
                        }

                    }

                    //@Override
                    void handleIMapBatchInvalidation(const std::vector<serialization::pimpl::Data> &keys) {
                        for (std::vector<serialization::pimpl::Data>::const_iterator it = keys.begin();
                             it != keys.end(); ++it) {
                            nearCache->remove(*it);
                        }
                    }

                private:
                    boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V, internal::adapter::IMapDataStructureAdapter<K, V> > > nearCache;
                };

                std::auto_ptr<protocol::codec::IAddListenerCodec> createNearCacheEntryListenerCodec() {
                    return std::auto_ptr<protocol::codec::IAddListenerCodec>(
                            new protocol::codec::MapAddNearCacheEntryListenerCodec(
                                    nearCache->getName(), EntryEventType::INVALIDATION, true));
                }

                void tryToPutNearCache(const serialization::pimpl::Data &key, boost::shared_ptr<V> response) {
                    try {
                        nearCache->put(key, response);
                    } catch (...) {
                            if (!keyStateMarker->tryUnmark(key)) {
                                invalidateNearCache(key);
                                keyStateMarker->forceUnmark(key);
                            }
                        throw;
                    }
                }

                void invalidateNearCache(const serialization::pimpl::Data &key) {
                    nearCache->remove(key);
                }

                const config::NearCacheConfig *nearCacheConfig;
                boost::shared_ptr<internal::nearcache::NearCache<
                        serialization::pimpl::Data, V, internal::adapter::IMapDataStructureAdapter<K, V> > > nearCache;
                internal::nearcache::impl::KeyStateMarker<serialization::pimpl::Data> *keyStateMarker;
                bool invalidateOnChange;

                // since we don't have atomic support in the project yet, using shared_ptr
                boost::shared_ptr<std::string> invalidationListenerId;

                boost::shared_ptr<client::impl::BaseEventHandler> invalidationHandler;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_NEARCACHEDCLIENTMAPPROXY_H_ */

