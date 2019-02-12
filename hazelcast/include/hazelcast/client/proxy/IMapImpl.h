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
#ifndef HAZELCAST_IMAP_IMPL
#define HAZELCAST_IMAP_IMPL

#include "hazelcast/client/EntryArray.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/Predicate.h"
#include "hazelcast/client/protocol/codec/MapExecuteWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeyCodec.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnAllKeysCodec.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/map/DataEntryView.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/MapPutCodec.h"
#include "hazelcast/client/protocol/codec/MapGetCodec.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class IdentifiedDataSerializable;
        }
        namespace proxy {
            class HAZELCAST_API IMapImpl : public ProxyImpl {
            protected:
                IMapImpl(const std::string &instanceName, spi::ClientContext *context);

                bool containsKey(const serialization::pimpl::Data &key);

                bool containsValue(const serialization::pimpl::Data &value);

                std::auto_ptr<serialization::pimpl::Data> getData(const serialization::pimpl::Data &key);

                std::auto_ptr<serialization::pimpl::Data> removeData(const serialization::pimpl::Data &key);

                bool remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                void removeAll(const serialization::pimpl::Data &predicateData);

                void deleteEntry(const serialization::pimpl::Data &key);

                virtual void flush();

                bool tryRemove(const serialization::pimpl::Data &key, int64_t timeoutInMillis);

                bool tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                            int64_t timeoutInMillis);

                std::auto_ptr<serialization::pimpl::Data>
                putData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                        int64_t ttlInMillis);

                void putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  int64_t ttlInMillis);

                std::auto_ptr<serialization::pimpl::Data>
                putIfAbsentData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                int64_t ttlInMillis);

                bool replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                             const serialization::pimpl::Data &newValue);

                std::auto_ptr<serialization::pimpl::Data>
                replaceData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                void set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, int64_t ttl);

                void lock(const serialization::pimpl::Data &key);

                void lock(const serialization::pimpl::Data &key, int64_t leaseTime);

                bool isLocked(const serialization::pimpl::Data &key);

                bool tryLock(const serialization::pimpl::Data &key, int64_t timeInMillis);

                void unlock(const serialization::pimpl::Data &key);

                void forceUnlock(const serialization::pimpl::Data &key);

                std::string addInterceptor(serialization::Portable &interceptor);

                std::string addInterceptor(serialization::IdentifiedDataSerializable &interceptor);

                virtual void removeInterceptor(const std::string &id);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                std::string
                addEntryListener(impl::BaseEventHandler *entryEventHandler, const query::Predicate &predicate,
                                 bool includeValue);

                virtual bool removeEntryListener(const std::string &registrationId);

                std::string
                addEntryListener(impl::BaseEventHandler *entryEventHandler, serialization::pimpl::Data &key,
                                 bool includeValue);

                std::auto_ptr<map::DataEntryView> getEntryViewData(const serialization::pimpl::Data &key);

                bool evict(const serialization::pimpl::Data &key);

                virtual void evictAll();

                EntryVector
                getAllData(const std::map<int, std::vector<serialization::pimpl::Data> > &partitionToKeyData);

                std::vector<serialization::pimpl::Data> keySetData();

                std::vector<serialization::pimpl::Data> keySetData(
                        const serialization::IdentifiedDataSerializable &predicate);

                std::vector<serialization::pimpl::Data>
                keySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetData();

                EntryVector entrySetData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                std::vector<serialization::pimpl::Data> valuesData();

                std::vector<serialization::pimpl::Data> valuesData(
                        const serialization::IdentifiedDataSerializable &predicate);

                EntryVector valuesForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                virtual void addIndex(const std::string &attribute, bool ordered);

                virtual int size();

                virtual bool isEmpty();

                virtual void putAllData(const std::map<int, EntryVector> &entries);

                virtual void clear();

                std::auto_ptr<serialization::pimpl::Data> executeOnKeyData(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                EntryVector executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                              const serialization::pimpl::Data &processor);

                template<typename EntryProcessor>
                EntryVector executeOnEntriesData(const EntryProcessor &entryProcessor) {
                    serialization::pimpl::Data processor = toData<EntryProcessor>(entryProcessor);

                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapExecuteOnAllKeysCodec::encodeRequest(
                            getName(), processor);

                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response =
                            invokeAndGetResult<EntryVector, protocol::codec::MapExecuteOnAllKeysCodec::ResponseParameters>(
                                    request);

                    return response;
                }

                template<typename EntryProcessor>
                EntryVector
                executeOnEntriesData(const EntryProcessor &entryProcessor, const query::Predicate &predicate) {
                    serialization::pimpl::Data processor = toData<EntryProcessor>(entryProcessor);
                    serialization::pimpl::Data predData = toData<serialization::IdentifiedDataSerializable>(predicate);
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapExecuteWithPredicateCodec::encodeRequest(getName(),
                                                                                                     processor,
                                                                                                     predData);

                    return invokeAndGetResult<EntryVector,
                            protocol::codec::MapExecuteWithPredicateCodec::ResponseParameters>(request);
                }

                template<typename K, typename V>
                std::pair<size_t, size_t> updateAnchor(EntryArray<K, V> &entries,
                                                       query::PagingPredicate<K, V> &predicate,
                                                       query::IterationType iterationType) {
                    if (0 == entries.size()) {
                        return std::pair<size_t, size_t>(0, 0);
                    }

                    const std::pair<size_t, std::pair<K *, V *> > *nearestAnchorEntry = predicate.getNearestAnchorEntry();
                    int nearestPage = (NULL == nearestAnchorEntry ? -1 : (int) nearestAnchorEntry->first);
                    size_t page = predicate.getPage();
                    size_t pageSize = predicate.getPageSize();
                    size_t begin = pageSize * (page - nearestPage - 1);
                    size_t size = entries.size();
                    if (begin > size) {
                        return std::pair<size_t, size_t>(0, 0);
                    }
                    size_t end = begin + pageSize;
                    if (end > size) {
                        end = size;
                    }

                    setAnchor(entries, predicate, nearestPage);

                    return std::pair<size_t, size_t>(begin, end);
                }

                template<typename K, typename V>
                static void
                setAnchor(EntryArray<K, V> &entries, query::PagingPredicate<K, V> &predicate, int nearestPage) {
                    if (0 == entries.size()) {
                        return;
                    }

                    size_t size = entries.size();
                    size_t pageSize = (size_t) predicate.getPageSize();
                    int page = (int) predicate.getPage();
                    for (size_t i = pageSize; i <= size && nearestPage < page; i += pageSize) {
                        std::auto_ptr<K> key = entries.releaseKey(i - 1);
                        std::auto_ptr<V> value = entries.releaseValue(i - 1);
                        std::pair<K *, V *> anchor(key.release(), value.release());
                        nearestPage++;
                        predicate.setAnchor((size_t) nearestPage, anchor);
                    }
                }

                virtual void onInitialize();

                boost::shared_ptr<spi::impl::ClientInvocationFuture>
                putAsyncInternalData(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, const int64_t *maxIdle,
                                     const util::concurrent::TimeUnit &maxIdleUnit,
                                     const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &valueData);

                boost::shared_ptr<spi::impl::ClientInvocationFuture>
                setAsyncInternalData(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, const int64_t *maxIdle,
                                     const util::concurrent::TimeUnit &maxIdleUnit,
                                     const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &valueData);
            private:
                class MapEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerWithPredicateMessageCodec(const std::string &name, bool includeValue,
                                                              int32_t listenerFlags,
                                                              serialization::pimpl::Data &predicate);

                    virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                    serialization::pimpl::Data predicate;
                };

                class MapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerMessageCodec(const std::string &name, bool includeValue, int32_t listenerFlags);

                    virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                };

                class MapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerToKeyCodec(const std::string &name, bool includeValue, int32_t listenerFlags,
                                               const serialization::pimpl::Data &key);

                    virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                    serialization::pimpl::Data key;
                };

                boost::shared_ptr<impl::ClientLockReferenceIdGenerator> lockReferenceIdGenerator;

                boost::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags);

                boost::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &predicate,
                                            int32_t listenerFlags);

                boost::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags, serialization::pimpl::Data &key);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_IMAP_IMPL */

