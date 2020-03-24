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

#include "hazelcast/client/mixedtype/HazelcastClient.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"
#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/mixedtype/IMap.h"
#include "hazelcast/client/mixedtype/Ringbuffer.h"
#include "hazelcast/client/mixedtype/ITopic.h"
#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/mixedtype/NearCachedClientMapProxy.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/map/impl/nearcache/InvalidationAwareWrapper.h"
#include "hazelcast/client/map/impl/nearcache/KeyStateMarker.h"
#include "hazelcast/client/map/impl/MapMixedTypeProxyFactory.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            MultiMap::MultiMap(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::MultiMapImpl(instanceName, context) {
            }

            std::vector<TypedData> MultiMap::keySet() {
                return toTypedDataCollection(proxy::MultiMapImpl::keySetData());
            }

            std::vector<TypedData> MultiMap::values() {
                return toTypedDataCollection(proxy::MultiMapImpl::valuesData());
            }

            std::vector<std::pair<TypedData, TypedData> > MultiMap::entrySet() {
                return toTypedDataEntrySet(proxy::MultiMapImpl::entrySetData());
            }

            int MultiMap::size() {
                return proxy::MultiMapImpl::size();
            }

            void MultiMap::clear() {
                proxy::MultiMapImpl::clear();
            }

            std::string MultiMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                spi::ClientClusterService &clusterService = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), clusterService, ss, listener, includeValue);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            bool MultiMap::removeEntryListener(const std::string &registrationId) {
                return proxy::MultiMapImpl::removeEntryListener(registrationId);
            }

            std::string IQueue::addItemListener(MixedItemListener &listener, bool includeValue) {
                spi::ClientClusterService &cs = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::QueueAddListenerCodec::AbstractEventHandler>(
                                getName(), cs, ss, listener);
                return proxy::IQueueImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool IQueue::removeItemListener(const std::string &registrationId) {
                return proxy::IQueueImpl::removeItemListener(registrationId);
            }

            int IQueue::remainingCapacity() {
                return proxy::IQueueImpl::remainingCapacity();
            }

            TypedData IQueue::take() {
                return poll(-1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements) {
                return drainTo(elements, -1);
            }

            size_t IQueue::drainTo(std::vector<TypedData> &elements, int64_t maxElements) {
                typedef std::vector<serialization::pimpl::Data> DATA_VECTOR;
                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                size_t numElements = 0;
                for (const DATA_VECTOR::value_type data : proxy::IQueueImpl::drainToData((size_t) maxElements)) {
                    elements.push_back(TypedData(std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(data)), serializationService));
                    ++numElements;
                }
                return numElements;
            }

            TypedData IQueue::poll() {
                return poll(0);
            }

            TypedData IQueue::poll(long timeoutInMillis) {
                return TypedData(proxy::IQueueImpl::pollData(timeoutInMillis), getContext().getSerializationService());
            }

            TypedData IQueue::peek() {
                return TypedData(proxy::IQueueImpl::peekData(), getContext().getSerializationService());
            }

            int IQueue::size() {
                return proxy::IQueueImpl::size();
            }

            bool IQueue::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IQueue::toArray() {
                return toTypedDataCollection(proxy::IQueueImpl::toArrayData());
            }

            void IQueue::clear() {
                proxy::IQueueImpl::clear();
            }

            IQueue::IQueue(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IQueueImpl(instanceName, context) {
            }

            IMap::IMap(std::shared_ptr<mixedtype::ClientMapProxy> proxy) : mapImpl(proxy) {
            }

            void IMap::removeAll(const query::Predicate &predicate) {
                mapImpl->removeAll(predicate);
            }

            void IMap::flush() {
                mapImpl->flush();
            }

            void IMap::removeInterceptor(const std::string &id) {
                mapImpl->removeInterceptor(id);
            }

            std::string IMap::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                return mapImpl->addEntryListener(listener, includeValue);
            }

            std::string
            IMap::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                   bool includeValue) {
                return mapImpl->addEntryListener(listener, predicate, includeValue);
            }

            bool IMap::removeEntryListener(const std::string &registrationId) {
                return mapImpl->removeEntryListener(registrationId);
            }

            void IMap::evictAll() {
                mapImpl->evictAll();
            }

            std::vector<TypedData> IMap::keySet() {
                return mapImpl->keySet();
            }

            std::vector<TypedData> IMap::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::keySet(const query::Predicate &predicate) {
                return mapImpl->keySet(predicate);
            }

            std::vector<TypedData> IMap::values() {
                return mapImpl->values();
            }

            std::vector<TypedData> IMap::values(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<TypedData> IMap::values(const query::Predicate &predicate) {
                return mapImpl->values(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet() {
                return mapImpl->entrySet();
            }

            std::vector<std::pair<TypedData, TypedData> >
            IMap::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->entrySet(predicate);
            }

            std::vector<std::pair<TypedData, TypedData> > IMap::entrySet(const query::Predicate &predicate) {
                return mapImpl->entrySet(predicate);
            }

            void IMap::addIndex(const std::string &attribute, bool ordered) {
                mapImpl->addIndex(attribute, ordered);
            }

            int IMap::size() {
                return mapImpl->size();
            }

            bool IMap::isEmpty() {
                return mapImpl->isEmpty();
            }

            void IMap::clear() {
                return mapImpl->clear();
            }

            void IMap::destroy() {
                mapImpl->destroy();
            }

            monitor::LocalMapStats &IMap::getLocalMapStats() {
                return mapImpl->getLocalMapStats();
            }

            Ringbuffer::Ringbuffer(const std::string &objectName, spi::ClientContext *context) : proxy::ProxyImpl(
                    "hz:impl:ringbufferService", objectName, context), bufferCapacity(-1) {
                partitionId = getPartitionId(toData(objectName));
            }

            Ringbuffer::Ringbuffer(const Ringbuffer &rhs) : proxy::ProxyImpl(rhs), partitionId(rhs.partitionId),
                                                            bufferCapacity(
                                                                    const_cast<Ringbuffer &>(rhs).bufferCapacity.load()) {
            }

            Ringbuffer::~Ringbuffer() {
            }

            int64_t Ringbuffer::capacity() {
                if (-1 == bufferCapacity) {
                    std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::encodeRequest(
                            getName());
                    bufferCapacity = invokeAndGetResult<int64_t, protocol::codec::RingbufferCapacityCodec::ResponseParameters>(
                            msg, partitionId);
                }
                return bufferCapacity;
            }

            int64_t Ringbuffer::size() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferSizeCodec::ResponseParameters>(msg,
                                                                                                             partitionId);
            }

            int64_t Ringbuffer::tailSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferTailSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::headSequence() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters>(
                        msg, partitionId);
            }

            int64_t Ringbuffer::remainingCapacity() {
                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::encodeRequest(
                        getName());
                return invokeAndGetResult<int64_t, protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters>(
                        msg, partitionId);
            }

            TypedData Ringbuffer::readOne(int64_t sequence) {
                checkSequence(sequence);

                std::unique_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadOneCodec::encodeRequest(
                        getName(), sequence);

                std::unique_ptr<serialization::pimpl::Data> itemData = invokeAndGetResult<
                        std::unique_ptr<serialization::pimpl::Data>, protocol::codec::RingbufferReadOneCodec::ResponseParameters>(
                        msg, partitionId);

                return TypedData(itemData, getContext().getSerializationService());
            }

            void Ringbuffer::checkSequence(int64_t sequence) {
                if (sequence < 0) {
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                            "Ringbuffer::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                         << sequence).build();
                }
            }

            std::string ITopic::addMessageListener(topic::MessageListener &listener) {
                client::impl::BaseEventHandler *topicEventHandler = new mixedtype::topic::impl::TopicEventHandlerImpl(
                        getName(), getContext().getClientClusterService(),
                        getContext().getSerializationService(),
                        listener);
                return proxy::ITopicImpl::addMessageListener(topicEventHandler);
            }

            bool ITopic::removeMessageListener(const std::string &registrationId) {
                return proxy::ITopicImpl::removeMessageListener(registrationId);
            }

            ITopic::ITopic(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ITopicImpl(instanceName, context) {
            }

            std::string ISet::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::SetAddListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            bool ISet::removeItemListener(const std::string &registrationId) {
                return proxy::ISetImpl::removeItemListener(registrationId);
            }

            int ISet::size() {
                return proxy::ISetImpl::size();
            }

            bool ISet::isEmpty() {
                return proxy::ISetImpl::isEmpty();
            }

            std::vector<TypedData> ISet::toArray() {
                return toTypedDataCollection(proxy::ISetImpl::toArrayData());
            }

            void ISet::clear() {
                proxy::ISetImpl::clear();
            }

            ISet::ISet(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ISetImpl(instanceName, context) {
            }

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
                    std::unique_ptr<client::impl::BaseEventHandler> invalidationHandler(
                            new ClientMapAddNearCacheEventHandler(nearCache));
                    addNearCacheInvalidateListener(invalidationHandler);
                }
            }

            //@Override
            bool NearCachedClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                std::shared_ptr<serialization::pimpl::Data> key = toShared(keyData);
                std::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    return internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT != cached;
                }

                return ClientMapProxy::containsKeyInternal(*key);
            }

            //@override
            std::shared_ptr<TypedData> NearCachedClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(keyData);
                std::shared_ptr<TypedData> cached = nearCache->get(key);
                if (cached.get() != NULL) {
                    if (internal::nearcache::NearCache<serialization::pimpl::Data, TypedData>::NULL_OBJECT == cached) {
                        return std::shared_ptr<TypedData>(
                                new TypedData(std::unique_ptr<serialization::pimpl::Data>(),
                                              getSerializationService()));
                    }
                    return cached;
                }

                bool marked = keyStateMarker->tryMark(*key);

                try {
                    std::shared_ptr<TypedData> value = ClientMapProxy::getInternal(*key);
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
            std::unique_ptr<serialization::pimpl::Data> NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> responseData = ClientMapProxy::removeInternal(key);
                    invalidateNearCache(key);
                    return responseData;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            //@Override
            bool NearCachedClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                try {
                    bool response = ClientMapProxy::removeInternal(key, value);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
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
                try {
                    ClientMapProxy::deleteInternal(key);
                    invalidateNearCache(key);
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            bool
            NearCachedClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &key, long timeoutInMillis) {
                try {
                    bool response = ClientMapProxy::tryRemoveInternal(key, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::tryPutInternal(const serialization::pimpl::Data &key,
                                                          const serialization::pimpl::Data &value,
                                                          long timeoutInMillis) {
                try {
                    bool response = ClientMapProxy::tryPutInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putInternal(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value,
                                                  long timeoutInMillis) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy::putInternal(key, value, timeoutInMillis);
                    invalidateNearCache(key);
                    return previousValue;
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            void NearCachedClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &key,
                                                                   const serialization::pimpl::Data &value,
                                                                   int64_t ttlInMillis) {
                try {
                    ClientMapProxy::tryPutTransientInternal(key, value, ttlInMillis);
                    invalidateNearCache(key);
                } catch (exception::IException &) {
                    invalidateNearCache(key);
                    throw;
                }
            }

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                          const serialization::pimpl::Data &valueData,
                                                          int64_t ttlInMillis) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> previousValue =
                            ClientMapProxy::putIfAbsentData(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                    return previousValue;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
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

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                                      const serialization::pimpl::Data &valueData) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> value =
                            proxy::IMapImpl::replaceData(keyData, valueData);
                    invalidateNearCache(keyData);
                    return value;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            void NearCachedClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       int64_t ttlInMillis) {
                try {
                    proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                    invalidateNearCache(keyData);
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            bool NearCachedClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                try {
                    bool evicted = proxy::IMapImpl::evict(keyData);
                    invalidateNearCache(keyData);
                    return evicted;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            EntryVector NearCachedClientMapProxy::getAllInternal(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData) {
                MARKER_MAP markers;
                try {
                    ClientMapProxy::PID_TO_KEY_MAP nonCachedPidToKeyMap;
                    EntryVector result = populateFromNearCache(pIdToKeyData, nonCachedPidToKeyMap, markers);

                    EntryVector responses = ClientMapProxy::getAllInternal(nonCachedPidToKeyMap);
                    for (const EntryVector::value_type &entry : responses) {
                        std::shared_ptr<serialization::pimpl::Data> key = ClientMapProxy::toShared(
                                entry.first);
                        std::shared_ptr<TypedData> value = std::shared_ptr<TypedData>(new TypedData(
                                std::unique_ptr<serialization::pimpl::Data>(
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

            std::unique_ptr<serialization::pimpl::Data>
            NearCachedClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                           const serialization::pimpl::Data &processor) {
                try {
                    std::unique_ptr<serialization::pimpl::Data> response =
                            ClientMapProxy::executeOnKeyData(keyData, processor);
                    invalidateNearCache(keyData);
                    return response;
                } catch (exception::IException &) {
                    invalidateNearCache(keyData);
                    throw;
                }
            }

            void
            NearCachedClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                try {
                    ClientMapProxy::putAllInternal(entries);
                    invalidateEntries(entries);
                } catch (exception::IException &) {
                    invalidateEntries(entries);
                    throw;
                }
            }

            void NearCachedClientMapProxy::invalidateEntries(const std::map<int, EntryVector> &entries) {
                for (std::map<int, EntryVector>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                    for (EntryVector::const_iterator entryIt = it->second.begin();
                         entryIt != it->second.end(); ++entryIt) {
                        invalidateNearCache(ClientMapProxy::toShared(entryIt->first));
                    }
                }
            }

            map::impl::nearcache::KeyStateMarker *NearCachedClientMapProxy::getKeyStateMarker() {
                return std::static_pointer_cast<
                        map::impl::nearcache::InvalidationAwareWrapper<serialization::pimpl::Data, TypedData> >(
                        nearCache)->
                        getKeyStateMarker();
            }

            void NearCachedClientMapProxy::addNearCacheInvalidateListener(
                    std::unique_ptr<client::impl::BaseEventHandler> &handler) {
                try {
                    invalidationListenerId = std::shared_ptr<std::string>(
                            new std::string(proxy::ProxyImpl::registerListener(
                                    createNearCacheEntryListenerCodec(), handler.release())));

                } catch (exception::IException &e) {
                    std::ostringstream out;
                    out << "-----------------\n Near Cache is not initialized!!! \n-----------------";
                    out << e.what();
                    getContext().getLogger().severe(out.str());
                }
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            NearCachedClientMapProxy::createNearCacheEntryListenerCodec() {
                int32_t listenerFlags = EntryEventType::INVALIDATION;
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new NearCacheEntryListenerMessageCodec(getName(), listenerFlags));
            }

            void NearCachedClientMapProxy::resetToUnmarkedState(std::shared_ptr<serialization::pimpl::Data> &key) {
                if (keyStateMarker->tryUnmark(*key)) {
                    return;
                }

                invalidateNearCache(key);
                keyStateMarker->forceUnmark(*key);
            }

            void NearCachedClientMapProxy::unmarkRemainingMarkedKeys(
                    std::map<std::shared_ptr<serialization::pimpl::Data>, bool> &markers) {
                for (std::map<std::shared_ptr<serialization::pimpl::Data>, bool>::const_iterator it = markers.begin();
                     it != markers.end(); ++it) {
                    if (it->second) {
                        keyStateMarker->forceUnmark(*it->first);
                    }
                }
            }

            void NearCachedClientMapProxy::tryToPutNearCache(std::shared_ptr<serialization::pimpl::Data> &keyData,
                                                             std::shared_ptr<TypedData> &response) {
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
                nearCache->invalidate(ClientMapProxy::toShared(key));
            }

            void NearCachedClientMapProxy::invalidateNearCache(std::shared_ptr<serialization::pimpl::Data> key) {
                nearCache->invalidate(key);
            }

            EntryVector
            NearCachedClientMapProxy::populateFromNearCache(const ClientMapProxy::PID_TO_KEY_MAP &pIdToKeyData,
                                                            PID_TO_KEY_MAP &nonCachedPidToKeyMap, MARKER_MAP &markers) {
                EntryVector result;

                for (const ClientMapProxy::PID_TO_KEY_MAP::value_type &partitionDatas : pIdToKeyData) {
                    typedef std::vector<std::shared_ptr<serialization::pimpl::Data> > SHARED_DATA_VECTOR;
                    SHARED_DATA_VECTOR nonCachedData;
                    for (const SHARED_DATA_VECTOR::value_type &keyData : partitionDatas.second) {
                        std::shared_ptr<TypedData> cached = nearCache->get(keyData);
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

            std::unique_ptr<protocol::ClientMessage>
            NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::encodeRequest(name, listenerFlags,
                                                                                         localOnly);
            }

            std::string NearCachedClientMapProxy::NearCacheEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
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

            IList::IList(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IListImpl(instanceName, context) {
            }

            std::string IList::addItemListener(MixedItemListener &listener, bool includeValue) {
                impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>(
                                getName(), (spi::ClientClusterService &) getContext().getClientClusterService(),
                                getContext().getSerializationService(), listener);
                return proxy::IListImpl::addItemListener(entryEventHandler, includeValue);
            }

            bool IList::removeItemListener(const std::string &registrationId) {
                return proxy::IListImpl::removeItemListener(registrationId);
            }

            int IList::size() {
                return proxy::IListImpl::size();
            }

            bool IList::isEmpty() {
                return size() == 0;
            }

            std::vector<TypedData> IList::toArray() {
                return toTypedDataCollection(proxy::IListImpl::toArrayData());
            }

            void IList::clear() {
                proxy::IListImpl::clear();
            }

            TypedData IList::get(int index) {
                return TypedData(proxy::IListImpl::getData(index), getContext().getSerializationService());
            }

            TypedData IList::remove(int index) {
                return TypedData(proxy::IListImpl::removeData(index), getContext().getSerializationService());
            }

            std::vector<TypedData> IList::subList(int fromIndex, int toIndex) {
                return toTypedDataCollection(proxy::IListImpl::subListData(fromIndex, toIndex));
            }

            ClientMapProxy::ClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::IMapImpl(instanceName, context) {
            }

            void ClientMapProxy::removeAll(const query::Predicate &predicate) {
                serialization::pimpl::Data predicateData = toData(predicate);

                removeAllInternal(predicateData);
            }

            void ClientMapProxy::flush() {
                proxy::IMapImpl::flush();
            }

            void ClientMapProxy::removeInterceptor(const std::string &id) {
                proxy::IMapImpl::removeInterceptor(id);
            }

            std::string ClientMapProxy::addEntryListener(MixedEntryListener &listener, bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, includeValue);
            }

            std::string
            ClientMapProxy::addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate,
                                             bool includeValue) {
                impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::MixedEntryEventHandler<protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                listener,
                                includeValue);
                return proxy::IMapImpl::addEntryListener(entryEventHandler, predicate, includeValue);
            }

            bool ClientMapProxy::removeEntryListener(const std::string &registrationId) {
                return proxy::IMapImpl::removeEntryListener(registrationId);
            }

            void ClientMapProxy::evictAll() {
                proxy::IMapImpl::evictAll();
            }

            std::vector<TypedData> ClientMapProxy::keySet() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, getContext().getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::keySet(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return keySet(*p);
            }

            std::vector<TypedData> ClientMapProxy::keySet(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> keys(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(new serialization::pimpl::Data(dataResult[i]));
                    keys[i] = TypedData(keyData, getContext().getSerializationService());
                }
                return keys;
            }

            std::vector<TypedData> ClientMapProxy::values() {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, getContext().getSerializationService());
                }
                return values;
            }

            std::vector<TypedData> ClientMapProxy::values(const serialization::IdentifiedDataSerializable &predicate) {
                const query::Predicate *p = (const query::Predicate *) (&predicate);
                return values(*p);
            }

            std::vector<TypedData> ClientMapProxy::values(const query::Predicate &predicate) {
                std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                size_t size = dataResult.size();
                std::vector<TypedData> values(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i]));
                    values[i] = TypedData(valueData, getContext().getSerializationService());
                }
                return values;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet() {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData();
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService), TypedData(valueData,
                                                                                                    serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> >
            ClientMapProxy::entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            std::vector<std::pair<TypedData, TypedData> > ClientMapProxy::entrySet(const query::Predicate &predicate) {
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                        predicate);
                size_t size = dataResult.size();
                std::vector<std::pair<TypedData, TypedData> > entries(size);
                for (size_t i = 0; i < size; ++i) {
                    std::unique_ptr<serialization::pimpl::Data> keyData(
                            new serialization::pimpl::Data(dataResult[i].first));
                    std::unique_ptr<serialization::pimpl::Data> valueData(
                            new serialization::pimpl::Data(dataResult[i].second));
                    serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
                    entries[i] = std::make_pair(TypedData(keyData, serializationService),
                                                TypedData(valueData, serializationService));
                }
                return entries;
            }

            void ClientMapProxy::addIndex(const std::string &attribute, bool ordered) {
                proxy::IMapImpl::addIndex(attribute, ordered);
            }

            int ClientMapProxy::size() {
                return proxy::IMapImpl::size();
            }

            bool ClientMapProxy::isEmpty() {
                return proxy::IMapImpl::isEmpty();
            }

            void ClientMapProxy::clear() {
                proxy::IMapImpl::clear();
            }

            serialization::pimpl::SerializationService &ClientMapProxy::getSerializationService() {
                return getContext().getSerializationService();
            }

            monitor::LocalMapStats &ClientMapProxy::getLocalMapStats() {
                return stats;
            }

            std::shared_ptr<TypedData> ClientMapProxy::getInternal(serialization::pimpl::Data &keyData) {
                std::unique_ptr<serialization::pimpl::Data> valueData = proxy::IMapImpl::getData(keyData);
                return std::shared_ptr<TypedData>(new TypedData(valueData, getContext().getSerializationService()));
            }

            bool ClientMapProxy::containsKeyInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::containsKey(keyData);
            }

            std::unique_ptr<serialization::pimpl::Data> ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::removeData(keyData);
            }

            bool ClientMapProxy::removeInternal(
                    const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::remove(keyData, valueData);
            }

            void ClientMapProxy::removeAllInternal(const serialization::pimpl::Data &predicateData) {
                return proxy::IMapImpl::removeAll(predicateData);
            }

            void ClientMapProxy::deleteInternal(const serialization::pimpl::Data &keyData) {
                proxy::IMapImpl::deleteEntry(keyData);
            }

            bool
            ClientMapProxy::tryRemoveInternal(const serialization::pimpl::Data &keyData, long timeoutInMillis) {
                return proxy::IMapImpl::tryRemove(keyData, timeoutInMillis);
            }

            bool ClientMapProxy::tryPutInternal(const serialization::pimpl::Data &keyData,
                                                const serialization::pimpl::Data &valueData,
                                                long timeoutInMillis) {
                return proxy::IMapImpl::tryPut(keyData, valueData, timeoutInMillis);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::putInternal(const serialization::pimpl::Data &keyData,
                                        const serialization::pimpl::Data &valueData, long timeoutInMillis) {
                return proxy::IMapImpl::putData(keyData, valueData, timeoutInMillis);
            }

            void ClientMapProxy::tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                         const serialization::pimpl::Data &valueData,
                                                         int64_t ttlInMillis) {
                proxy::IMapImpl::tryPut(keyData, valueData, ttlInMillis);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                                const serialization::pimpl::Data &valueData,
                                                int64_t ttlInMillis) {
                return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                       const serialization::pimpl::Data &valueData,
                                                       const serialization::pimpl::Data &newValueData) {
                return proxy::IMapImpl::replace(keyData, valueData, newValueData);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::replaceInternal(const serialization::pimpl::Data &keyData,
                                            const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::replaceData(keyData, valueData);

            }

            void ClientMapProxy::setInternal(const serialization::pimpl::Data &keyData,
                                             const serialization::pimpl::Data &valueData,
                                             int64_t ttlInMillis) {
                proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
            }

            bool ClientMapProxy::evictInternal(const serialization::pimpl::Data &keyData) {
                return proxy::IMapImpl::evict(keyData);
            }

            EntryVector ClientMapProxy::getAllInternal(const PID_TO_KEY_MAP &partitionToKeyData) {
                std::map<int, std::vector<serialization::pimpl::Data> > datas;
                for (PID_TO_KEY_MAP::const_iterator it = partitionToKeyData.begin();
                     it != partitionToKeyData.end(); ++it) {
                    const std::vector<std::shared_ptr<serialization::pimpl::Data> > &valueDatas = it->second;
                    for (std::vector<std::shared_ptr<serialization::pimpl::Data> >::const_iterator valueIt = valueDatas.begin();
                         valueIt != valueDatas.end(); ++valueIt) {
                        datas[it->first].push_back(*(*valueIt));
                    }
                }
                return proxy::IMapImpl::getAllData(datas);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                                 const serialization::pimpl::Data &processor) {
                return proxy::IMapImpl::executeOnKeyData(keyData, processor);
            }

            std::unique_ptr<serialization::pimpl::Data>
            ClientMapProxy::submitToKeyDecoder(protocol::ClientMessage &response) {
                return protocol::codec::MapExecuteOnKeyCodec::ResponseParameters::decode(response).response;
            }

            void
            ClientMapProxy::putAllInternal(const std::map<int, EntryVector> &entries) {
                proxy::IMapImpl::putAllData(entries);
            }

            namespace impl {
                HazelcastClientImpl::HazelcastClientImpl(client::impl::HazelcastClientInstanceImpl &client) : client(
                        client) {
                }

                HazelcastClientImpl::~HazelcastClientImpl() {
                }

                IMap HazelcastClientImpl::getMap(const std::string &name) {
                    map::impl::MapMixedTypeProxyFactory factory(&client.clientContext);
                    std::shared_ptr<spi::ClientProxy> proxy =
                            client.getDistributedObjectForService("hz:impl:mapService", name, factory);
                    return IMap(std::static_pointer_cast<ClientMapProxy>(proxy));
                }

                MultiMap HazelcastClientImpl::getMultiMap(const std::string &name) {
                    return client.getDistributedObject<MultiMap>(name);
                }

                IQueue HazelcastClientImpl::getQueue(const std::string &name) {
                    return client.getDistributedObject<IQueue>(name);
                }

                ISet HazelcastClientImpl::getSet(const std::string &name) {
                    return client.getDistributedObject<ISet>(name);
                }

                IList HazelcastClientImpl::getList(const std::string &name) {
                    return client.getDistributedObject<IList>(name);
                }

                ITopic HazelcastClientImpl::getTopic(const std::string &name) {
                    return client.getDistributedObject<ITopic>(name);
                }

                Ringbuffer HazelcastClientImpl::getRingbuffer(
                        const std::string &instanceName) {
                    return client.getDistributedObject<Ringbuffer>(instanceName);
                }

            }
        }
    }
}
