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
#ifndef HAZELCAST_CLIENT_NEARCACHEDMIXEDMAP_H_
#define HAZELCAST_CLIENT_NEARCACHEDMIXEDMAP_H_

#include <hazelcast/client/internal/nearcache/NearCache.h>
#include <hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h>
#include "hazelcast/client/MixedMap.h"
#include "hazelcast/client/config/NearCacheConfig.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                namespace nearcache {
                    class KeyStateMarker;
                }
            }
        }

        /**
        * Concurrent, distributed, observable and queryable map client.
        *
        * Notice that this class have a private constructor.
        * You can access get an IMap in the following way
        *
        *      ClientConfig clientConfig;
        *      HazelcastClient client(clientConfig);
        *      MixedMap map = client.getMixedMap("aKey");
        *
        */
        class HAZELCAST_API NearCachedMixedMap : public MixedMap {
        public:
            NearCachedMixedMap(const std::string &instanceName, spi::ClientContext *context,
                                           const hazelcast::client::config::MixedNearCacheConfig &config);

            virtual monitor::LocalMapStats &getLocalMapStats();

        protected:
            //@override
            void onInitialize();

            //@Override
            bool containsKeyInternal(const serialization::pimpl::Data &keyData);

            //@override
            boost::shared_ptr<TypedData> getInternal(serialization::pimpl::Data &keyData);

            //@Override
            virtual std::auto_ptr<serialization::pimpl::Data> removeInternal(
                    const serialization::pimpl::Data &key);

            //@Override
            virtual bool removeInternal(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

            virtual void removeAllInternal(const serialization::pimpl::Data &predicateData);

            virtual void deleteInternal(const serialization::pimpl::Data &key);

            virtual bool tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis);

            virtual bool tryPutInternal(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        long timeoutInMillis);

            virtual std::auto_ptr<serialization::pimpl::Data> putInternal(const serialization::pimpl::Data &key,
                                                                          const serialization::pimpl::Data &value,
                                                                          long timeoutInMillis);

            virtual void tryPutTransientInternal(const serialization::pimpl::Data &key,
                                                 const serialization::pimpl::Data &value, int ttlInMillis);

            virtual std::auto_ptr<serialization::pimpl::Data> putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                                                  const serialization::pimpl::Data &valueData,
                                                                                  int ttlInMillis);

            virtual bool replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                               const serialization::pimpl::Data &valueData,
                                               const serialization::pimpl::Data &newValueData);

            virtual std::auto_ptr<serialization::pimpl::Data> replaceInternal(const serialization::pimpl::Data &keyData,
                                                                              const serialization::pimpl::Data &valueData);

            virtual void setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                                     int ttlInMillis);

            virtual bool evictInternal(const serialization::pimpl::Data &keyData);

            EntryVector getAllInternal(MixedMap::PID_TO_KEY_MAP &pIdToKeyData);

            virtual std::auto_ptr<serialization::pimpl::Data>
            executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                 const serialization::pimpl::Data &processor);

            virtual void
            putAllInternal(const std::map<int, EntryVector> &entries);
        private:
            typedef std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> MARKER_MAP;

            map::impl::nearcache::KeyStateMarker *getKeyStateMarker();

            void addNearCacheInvalidateListener(std::auto_ptr<client::impl::BaseEventHandler> handler);

            class ClientMapAddNearCacheEventHandler
                    : public protocol::codec::MapAddNearCacheEntryListenerCodec::AbstractEventHandler {
            public:
                ClientMapAddNearCacheEventHandler(
                        const boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, TypedData> > &cache)
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
                boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, TypedData> > nearCache;
            };

            std::auto_ptr<protocol::codec::IAddListenerCodec> createNearCacheEntryListenerCodec();

            void resetToUnmarkedState(boost::shared_ptr<serialization::pimpl::Data> &key);

            void unmarkRemainingMarkedKeys(std::map<boost::shared_ptr<serialization::pimpl::Data>, bool> &markers);

            void tryToPutNearCache(boost::shared_ptr<serialization::pimpl::Data> &keyData,
                                   boost::shared_ptr<TypedData> &response);

            /**
             * This method modifies the key Data internal pointer although it is marked as const
             * @param key The key for which to invalidate the near cache
             */
            void invalidateNearCache(const serialization::pimpl::Data &key);

            void invalidateNearCache(boost::shared_ptr<serialization::pimpl::Data> key);


        private:
            bool cacheLocalEntries;
            bool invalidateOnChange;
            map::impl::nearcache::KeyStateMarker *keyStateMarker;
            const config::MixedNearCacheConfig &nearCacheConfig;
            boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, TypedData> > nearCache;
            // since we don't have atomic support in the project yet, using shared_ptr
            boost::shared_ptr<std::string> invalidationListenerId;

            /**
             * Populates the result from near cache if the data was near cached for the key. Also removes, the cached
             * entries from pIdToKeyData.
             * @param pIdToKeyData Partition Id to key data vector mapping
             * @param markers The markers to be used
             * @return The found cached data entries.
             */
            EntryVector populateFromNearCache(PID_TO_KEY_MAP &pIdToKeyData, MARKER_MAP &markers);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_NEARCACHEDMIXEDMAP_H_ */

