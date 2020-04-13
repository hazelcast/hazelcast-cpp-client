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
#ifndef HAZELCAST_CLIENT_PROXY_CLIENTREPLICATEDMAPPROXY_H_
#define HAZELCAST_CLIENT_PROXY_CLIENTREPLICATEDMAPPROXY_H_

#include <stdlib.h>

#include "hazelcast/client/ReplicatedMap.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/map/impl/DataAwareEntryEvent.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/impl/DataArrayImpl.h"
#include "hazelcast/client/impl/LazyEntryArrayImpl.h"

#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4250) //for warning class1' : inherits 'class2::member' via dominance
#endif

namespace hazelcast {
    namespace client {
        namespace proxy {
            /**
             * The replicated map client side proxy implementation proxying all requests to a member node
             *
             * @param <K> key type
             * @param <V> value type
             */
            template<typename K, typename V>
            class ClientReplicatedMapProxy : public ReplicatedMap<K, V>, public proxy::ProxyImpl {
            public:
                static const std::string SERVICE_NAME;

                ClientReplicatedMapProxy(const std::string &objectName, spi::ClientContext *context) : proxy::ProxyImpl(
                        SERVICE_NAME, objectName, context) {
                }

                std::shared_ptr<V> put(const K &key, const V &value, int64_t ttl) {
                    std::shared_ptr<serialization::pimpl::Data> keyData;
                    try {
                        serialization::pimpl::Data valueData = toData<V>(value);
                        auto keyData = toSharedData<K>(key);
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapPutCodec::encodeRequest(
                                name, *keyData, valueData, ttl);
                        std::unique_ptr<serialization::pimpl::Data> response = invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapPutCodec::ResponseParameters>(
                                request, *keyData);

                        invalidate(keyData);

                        return toSharedObject<V>(response);
                    } catch (...) {
                        invalidate(keyData);
                        throw;
                    }
                }

                virtual int32_t size() {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapSizeCodec::encodeRequest(
                            name);
                    return invokeAndGetResult<int32_t, protocol::codec::ReplicatedMapSizeCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                virtual bool isEmpty() {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapIsEmptyCodec::encodeRequest(
                            name);
                    return invokeAndGetResult<bool, protocol::codec::ReplicatedMapIsEmptyCodec::ResponseParameters>(
                            request, targetPartitionId);
                }

                virtual bool containsKey(const K &key) {
                    serialization::pimpl::Data keyData = toData<K>(key);
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapContainsKeyCodec::encodeRequest(
                            name, keyData);
                    return invokeAndGetResult<bool, protocol::codec::ReplicatedMapContainsKeyCodec::ResponseParameters>(
                            request, keyData);
                }

                virtual bool containsValue(const V &value) {
                    serialization::pimpl::Data valueData = toData<V>(value);
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapContainsValueCodec::encodeRequest(
                            name, valueData);
                    return invokeAndGetResult<bool, protocol::codec::ReplicatedMapContainsKeyCodec::ResponseParameters>(
                            request, valueData);
                }

                virtual std::shared_ptr<V> get(const K &key) {
                    std::shared_ptr<V> cachedValue = getCachedValue(key);
                    if (cachedValue.get() != NULL) {
                        return cachedValue;
                    }

                    std::shared_ptr<serialization::pimpl::Data> keyData;
                    try {
                        keyData = toShared(toData(key));
                        // TODO: Change to reservation model as in Java when near cache impl is updated
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapGetCodec::encodeRequest(
                                name, *keyData);
                        std::unique_ptr<serialization::pimpl::Data> result = invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapGetCodec::ResponseParameters>(
                                request, *keyData);

                        if (!result.get()) {
                            return std::shared_ptr<V>();
                        }

                        std::shared_ptr<V> value = toSharedObject<V>(result);

                        if (!value.get()) {
                            return std::shared_ptr<V>();
                        }

                        std::shared_ptr<internal::nearcache::NearCache<Data, V> > cache = nearCache.get();
                        if (cache.get()) {
                            cache->put(keyData, value);
                        }
                        return value;
                    } catch (exception::IException &) {
                        invalidate(keyData);
                        throw;
                    }
                }

                virtual std::shared_ptr<V> put(const K &key, const V &value) {
                    return put(key, value, (int64_t) 0);
                }

                virtual std::shared_ptr<V> remove(const K &key) {
                    std::shared_ptr<serialization::pimpl::Data> keyData;

                    try {
                        keyData = toShared(toData(key));
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapRemoveCodec::encodeRequest(
                                name, *keyData);
                        std::unique_ptr<serialization::pimpl::Data> result = invokeAndGetResult<std::unique_ptr<serialization::pimpl::Data>, protocol::codec::ReplicatedMapRemoveCodec::ResponseParameters>(
                                request, *keyData);

                        invalidate(keyData);

                        return toSharedObject<V>(result);
                    } catch (...) {
                        invalidate(keyData);
                        throw;
                    }
                }

                virtual void putAll(const std::map<K, V> &entries) {
                    EntryVector dataEntries;
                    try {
                        dataEntries = toDataEntries<K, V>(entries);

                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapPutAllCodec::encodeRequest(
                                name, dataEntries);
                        invoke(request);

                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            for (EntryVector::const_iterator it = dataEntries.begin(); it != dataEntries.end(); ++it) {
                                invalidate(toShared(it->first));
                            }
                        }
                    } catch (...) {
                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            for (EntryVector::const_iterator it = dataEntries.begin(); it != dataEntries.end(); ++it) {
                                invalidate(toShared(it->first));
                            }
                        }

                        throw;
                    }
                }

                virtual void clear() {
                    try {
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapClearCodec::encodeRequest(
                                name);
                        invoke(request);

                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            cache->clear();
                        }
                    } catch (...) {
                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            cache->clear();
                        }

                        throw;
                    }
                }

                virtual bool removeEntryListener(const std::string &registrationId) {
                    return deregisterListener(registrationId);
                }

                virtual std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener) {
                    util::Preconditions::isNotNull(listener, "listener");
                    std::shared_ptr<spi::EventHandler<protocol::ClientMessage> > handler = createHandler(listener);
                    return registerListener(createEntryListenerCodec(name), handler);
                }

                virtual std::string
                addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener, const K &key) {
                    util::Preconditions::isNotNull(listener, "listener");
                    std::shared_ptr<serialization::pimpl::Data> keyData = toShared(toData<K>(key));
                    std::shared_ptr<spi::EventHandler<protocol::ClientMessage> > handler = createHandler(listener);
                    return registerListener(createEntryListenerToKeyCodec(keyData), handler);
                }

                virtual const std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener,
                                                           const query::Predicate &predicate) {
                    util::Preconditions::isNotNull(listener, "listener");
                    std::shared_ptr<serialization::pimpl::Data> predicateData = toShared(
                            toData<serialization::IdentifiedDataSerializable>(predicate));
                    std::shared_ptr<spi::EventHandler<protocol::ClientMessage> > handler = createHandler(listener);
                    return registerListener(createEntryListenerWithPredicateCodec(predicateData), handler);
                }

                virtual std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener,
                                                     const query::Predicate &predicate,
                                                     const K &key) {
                    util::Preconditions::isNotNull(listener, "listener");
                    util::Preconditions::isNotNull(listener, "predicate");
                    std::shared_ptr<serialization::pimpl::Data> predicateData = toShared(
                            toData<serialization::IdentifiedDataSerializable>(predicate));
                    std::shared_ptr<serialization::pimpl::Data> keyData = toShared(toData<K>(key));
                    std::shared_ptr<spi::EventHandler<protocol::ClientMessage> > handler = createHandler(listener);
                    return registerListener(createEntryListenerToKeyWithPredicateCodec(keyData, predicateData),
                                            handler);
                }

                virtual std::shared_ptr<DataArray<K> > keySet() {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapKeySetCodec::encodeRequest(
                            name);
                    protocol::codec::ReplicatedMapKeySetCodec::ResponseParameters result = protocol::codec::ReplicatedMapKeySetCodec::ResponseParameters::decode(
                            invokeOnPartition(request, targetPartitionId));
                    return std::shared_ptr<DataArray<K> >(
                            new impl::DataArrayImpl<K>(result.response, getContext().getSerializationService()));
                }

                virtual std::shared_ptr<DataArray<V> > values() {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapValuesCodec::encodeRequest(
                            name);
                    protocol::codec::ReplicatedMapValuesCodec::ResponseParameters result = protocol::codec::ReplicatedMapValuesCodec::ResponseParameters::decode(
                            invokeOnPartition(request, targetPartitionId));
                    return std::shared_ptr<DataArray<V> >(
                            new impl::DataArrayImpl<V>(result.response, getContext().getSerializationService()));
                }

                virtual std::shared_ptr<LazyEntryArray<K, V> > entrySet() {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ReplicatedMapEntrySetCodec::encodeRequest(
                            name);
                    protocol::codec::ReplicatedMapEntrySetCodec::ResponseParameters result = protocol::codec::ReplicatedMapEntrySetCodec::ResponseParameters::decode(
                            invokeOnPartition(request, targetPartitionId));
                    return std::shared_ptr<LazyEntryArray<K, V> >(
                            new impl::LazyEntryArrayImpl<K, V>(result.response, getContext().getSerializationService()));
                }

                /**
                 * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor near cache statistics.
                 *
                 * This method is implemented for testing purposes.
                 *
                 * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                 */
                monitor::NearCacheStats *getNearCacheStats() {
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                    if (!cache.get()) {
                        return NULL;
                    }

                    return &cache->getNearCacheStats();
                }

            protected:
                virtual void onInitialize() {
                    ProxyImpl::onInitialize();

                    int partitionCount = getContext().getPartitionService().getPartitionCount();
                    targetPartitionId = rand() % partitionCount;

                    initNearCache();
                }

                virtual void postDestroy() {
                    try {
                        if (nearCache.get() != NULL) {
                            removeNearCacheInvalidationListener();
                            getContext().getNearCacheManager().destroyNearCache(name);
                        }

                        ClientProxy::postDestroy();
                    } catch (...) {
                        ClientProxy::postDestroy();
                        throw;
                    }

                }

            private:
                class NearCacheInvalidationListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    NearCacheInvalidationListenerMessageCodec(const std::string &name) : name(name) {}

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::encodeRequest(name, false,
                                                                                                           localOnly);
                    }

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
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
                                                                                const std::shared_ptr<Data> &keyData,
                                                                                const std::shared_ptr<Data> &predicateData)
                            : name(name), keyData(keyData), predicateData(predicateData) {}

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::encodeRequest(
                                name, *keyData, *predicateData, localOnly);
                    }

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                    const std::shared_ptr<serialization::pimpl::Data> keyData;
                    const std::shared_ptr<serialization::pimpl::Data> predicateData;
                };

                class ReplicatedMapAddEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerWithPredicateMessageCodec(const std::string &name,
                                                                           const std::shared_ptr<Data> &keyData)
                            : name(name),
                              predicateData(keyData) {}

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerWithPredicateCodec::encodeRequest(name,
                                                                                                               *predicateData,
                                                                                                               localOnly);
                    }

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                    const std::shared_ptr<serialization::pimpl::Data> predicateData;
                };

                class ReplicatedMapAddEntryListenerToKeyMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapAddEntryListenerToKeyMessageCodec(const std::string &name,
                                                                   const std::shared_ptr<Data> &keyData) : name(name),
                                                                                                             keyData(keyData) {}

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyCodec::encodeRequest(name, *keyData,
                                                                                                       localOnly);
                    }

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                    const std::shared_ptr<serialization::pimpl::Data> keyData;
                };

                class ReplicatedMapListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    ReplicatedMapListenerMessageCodec(const std::string &name) : name(name) {}

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerCodec::encodeRequest(name, localOnly);
                    }

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const {
                        return protocol::codec::ReplicatedMapAddEntryListenerCodec::ResponseParameters::decode(
                                std::move(responseMessage)).response;
                    }

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::encodeRequest(name,
                                                                                                     realRegistrationId);
                    }

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const {
                        return protocol::codec::ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(
                                std::move(clientMessage)).response;
                    }

                private:
                    std::string name;
                };

                class ReplicatedMapEventHandler
                        : public protocol::codec::ReplicatedMapAddEntryListenerCodec::AbstractEventHandler {
                public:
                    ReplicatedMapEventHandler(const std::string &name,
                                              const std::shared_ptr<EntryListener<K, V> > &listener,
                                              spi::ClientContext &clientContext) : name(name), listener(listener),
                                                                                   clientContext(clientContext) {}

                    virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                     std::unique_ptr<serialization::pimpl::Data> &value,
                                                     std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                     std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                     const int32_t &eventType, const std::string &uuid,
                                                     const int32_t &numberOfAffectedEntries) {
                        std::shared_ptr<Member> member = clientContext.getClientClusterService().getMember(uuid);

                        const EntryEventType &type = EntryEventType((EntryEventType::Type) eventType);
                        map::impl::DataAwareEntryEvent<K, V> entryEvent(name, *member, type,
                                                                        std::shared_ptr<serialization::pimpl::Data>(
                                                                                std::move(key)),
                                                                        std::shared_ptr<serialization::pimpl::Data>(
                                                                                std::move(value)),
                                                                        std::shared_ptr<serialization::pimpl::Data>(
                                                                                std::move(oldValue)),
                                                                        std::shared_ptr<serialization::pimpl::Data>(
                                                                                std::move(mergingValue)),
                                                                        clientContext.getSerializationService());
                        switch (eventType) {
                            case EntryEventType::ADDED:
                                listener->entryAdded(entryEvent);
                                break;
                            case EntryEventType::REMOVED:
                                listener->entryRemoved(entryEvent);
                                break;
                            case EntryEventType::UPDATED:
                                listener->entryUpdated(entryEvent);
                                break;
                            case EntryEventType::EVICTED:
                                listener->entryEvicted(entryEvent);
                                break;
                            case EntryEventType::CLEAR_ALL:
                                listener->mapCleared(MapEvent(*member, type, name, numberOfAffectedEntries));
                                break;
                            default:
                                throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                        "ReplicatedMapEventHandler::handleEntryEventV10") << "Not a known event type: "
                                                                                          << eventType).build();
                        }

                    }

                private:
                    std::string name;
                    const std::shared_ptr<EntryListener<K, V> > listener;
                    spi::ClientContext &clientContext;
                };

                class ReplicatedMapAddNearCacheEventHandler
                        : public protocol::codec::ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler {
                public:

                    ReplicatedMapAddNearCacheEventHandler(
                            util::Sync<std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > > &nearCache,
                            SerializationService &serializationService) : nearCache(nearCache),
                                                                          serializationService(serializationService) {}

                    void beforeListenerRegister() {
                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            cache->clear();
                        }
                    }

                    void onListenerRegister() {
                        std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                        if (cache.get() != NULL) {
                            cache->clear();
                        }
                    }

                    void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &dataKey,
                                             std::unique_ptr<serialization::pimpl::Data> &value,
                                             std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                             std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                             const int32_t &eventType, const std::string &uuid,
                                             const int32_t &numberOfAffectedEntries) {
                        switch (eventType) {
                            case EntryEventType::ADDED:
                            case EntryEventType::REMOVED:
                            case EntryEventType::UPDATED:
                            case EntryEventType::EVICTED: {
                                nearCache.get()->invalidate(std::shared_ptr<serialization::pimpl::Data>(std::move(dataKey)));
                                break;
                            }
                            case EntryEventType::CLEAR_ALL:
                                nearCache.get()->clear();
                                break;
                            default:
                                throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                        "ReplicatedMapAddNearCacheEventHandler::handleEntryEventV10")
                                        << "Not a known event type " << eventType).build();
                        }
                    }

                private:
                    util::Sync<std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > > &nearCache;
                    serialization::pimpl::SerializationService &serializationService;
                };

                std::shared_ptr<spi::EventHandler<protocol::ClientMessage> >
                createHandler(const std::shared_ptr<EntryListener<K, V> > &listener) {
                    return std::shared_ptr<spi::EventHandler<protocol::ClientMessage> >(
                            new ReplicatedMapEventHandler(getName(), listener, getContext()));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec> createEntryListenerCodec(const std::string name) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapListenerMessageCodec(name));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyCodec(const std::shared_ptr<serialization::pimpl::Data> &keyData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyMessageCodec(name, keyData));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerWithPredicateCodec(
                        const std::shared_ptr<serialization::pimpl::Data> &predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerWithPredicateMessageCodec(name, predicateData));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createEntryListenerToKeyWithPredicateCodec(const std::shared_ptr<serialization::pimpl::Data> &keyData,
                                                           const std::shared_ptr<serialization::pimpl::Data> &predicateData) {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(name, keyData,
                                                                                            predicateData));
                }

                std::shared_ptr<spi::impl::ListenerMessageCodec> createNearCacheInvalidationListenerCodec() {
                    return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                            new NearCacheInvalidationListenerMessageCodec(name));
                }

                void registerInvalidationListener() {
                    try {
                        invalidationListenerId = addNearCacheInvalidationListener(
                                std::shared_ptr<spi::EventHandler<protocol::ClientMessage> >(
                                        new ReplicatedMapAddNearCacheEventHandler(nearCache,
                                                                                  getContext().getSerializationService())));
                    } catch (exception::IException &e) {
                        getContext().getLogger().severe("-----------------\nNear Cache is not initialized!\n-----------------" , e);
                    }
                }

                std::string addNearCacheInvalidationListener(
                        const std::shared_ptr<spi::EventHandler<protocol::ClientMessage> > handler) {
                    return registerListener(createNearCacheInvalidationListenerCodec(), handler);
                }

                void initNearCache() {
                    std::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig = getContext().getClientConfig().template getNearCacheConfig<K, V>(
                            name);
                    if (nearCacheConfig.get() != NULL) {
                        nearCache = getContext().getNearCacheManager().template getOrCreateNearCache<K, V, serialization::pimpl::Data>(
                                name, *nearCacheConfig);
                        if (nearCacheConfig->isInvalidateOnChange()) {
                            registerInvalidationListener();
                        }
                    }

                }

                void removeNearCacheInvalidationListener() {
                    std::string listenerId = invalidationListenerId.get();
                    if (nearCache.get() != NULL && !listenerId.empty()) {
                        getContext().getClientListenerService().deregisterListener(listenerId);
                    }
                }

                void invalidate(const std::shared_ptr<serialization::pimpl::Data> &key) {
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                    if (cache.get() == NULL) {
                        return;
                    }
                    cache->invalidate(key);
                }

                std::shared_ptr<V> getCachedValue(const K &key) {
                    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > cache = nearCache.get();
                    if (cache.get() == NULL) {
                        // TODO Check to see if Java NOT_CACHED object impl. is possible
                        return std::shared_ptr<V>();
                    }

                    return cache->get(toSharedData<K>(key));
                }

                int targetPartitionId;

                util::Sync<std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, V> > > nearCache;
                util::Sync<std::string> invalidationListenerId;
            };

            template<typename K, typename V>
            const std::string ClientReplicatedMapProxy<K, V>::SERVICE_NAME = "hz:impl:replicatedMapService";
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_PROXY_CLIENTREPLICATEDMAPPROXY_H_
