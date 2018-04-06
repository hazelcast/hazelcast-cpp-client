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
//
// Created by sancar koyunlu on 29/09/14.
//

#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/spi/ClientListenerService.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/util/Util.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/MapPutCodec.h"
#include "hazelcast/client/protocol/codec/MapGetCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MapReplaceCodec.h"
#include "hazelcast/client/protocol/codec/MapReplaceIfSameCodec.h"
#include "hazelcast/client/protocol/codec/MapContainsKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapContainsValueCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveIfSameCodec.h"
#include "hazelcast/client/protocol/codec/MapDeleteCodec.h"
#include "hazelcast/client/protocol/codec/MapFlushCodec.h"
#include "hazelcast/client/protocol/codec/MapTryRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MapTryPutCodec.h"
#include "hazelcast/client/protocol/codec/MapPutTransientCodec.h"
#include "hazelcast/client/protocol/codec/MapPutIfAbsentCodec.h"
#include "hazelcast/client/protocol/codec/MapSetCodec.h"
#include "hazelcast/client/protocol/codec/MapLockCodec.h"
#include "hazelcast/client/protocol/codec/MapTryLockCodec.h"
#include "hazelcast/client/protocol/codec/MapIsLockedCodec.h"
#include "hazelcast/client/protocol/codec/MapUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MapForceUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MapAddInterceptorCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveInterceptorCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapGetEntryViewCodec.h"
#include "hazelcast/client/protocol/codec/MapEvictCodec.h"
#include "hazelcast/client/protocol/codec/MapEvictAllCodec.h"
#include "hazelcast/client/protocol/codec/MapLoadAllCodec.h"
#include "hazelcast/client/protocol/codec/MapKeySetCodec.h"
#include "hazelcast/client/protocol/codec/MapGetAllCodec.h"
#include "hazelcast/client/protocol/codec/MapValuesCodec.h"
#include "hazelcast/client/protocol/codec/MapEntrySetCodec.h"
#include "hazelcast/client/protocol/codec/MapKeySetWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapValuesWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapEntriesWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapAddIndexCodec.h"
#include "hazelcast/client/protocol/codec/MapSizeCodec.h"
#include "hazelcast/client/protocol/codec/MapIsEmptyCodec.h"
#include "hazelcast/client/protocol/codec/MapPutAllCodec.h"
#include "hazelcast/client/protocol/codec/MapClearCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnAllKeysCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapValuesWithPagingPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapKeySetWithPagingPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapEntriesWithPagingPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeysCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveAllCodec.h"

#include <climits>

namespace hazelcast {
    namespace client {
        namespace proxy {
            IMapImpl::IMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instanceName, context) {
            }

            bool IMapImpl::containsKey(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapContainsKeyCodec::RequestParameters::encode(getName(), key,
                                                                                        util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapContainsKeyCodec::ResponseParameters>(request,
                                                                                                          key);
            }

            bool IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapContainsValueCodec::RequestParameters::encode(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MapContainsValueCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::getData(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapGetCodec::ResponseParameters>(
                        request, key);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::removeData(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveIfSameCodec::RequestParameters::encode(getName(), key, value,
                                                                                         util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapRemoveIfSameCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            void IMapImpl::removeAll(const serialization::pimpl::Data &predicateData) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveAllCodec::RequestParameters::encode(getName(), predicateData);

                invoke(request);
            }

            void IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapDeleteCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::flush() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapFlushCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            bool IMapImpl::tryRemove(const serialization::pimpl::Data &key, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryRemoveCodec::RequestParameters::encode(getName(), key,
                                                                                      util::getThreadId(),
                                                                                      timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryRemoveCodec::ResponseParameters>(request, key);
            }

            bool IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryPutCodec::RequestParameters::encode(getName(), key, value,
                                                                                   util::getThreadId(),
                                                                                   timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryPutCodec::ResponseParameters>(request, key);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::putData(const serialization::pimpl::Data &key,
                                                                        const serialization::pimpl::Data &value,
                                                                        long ttlInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutCodec::RequestParameters::encode(getName(), key, value,
                                                                                util::getThreadId(),
                                                                                ttlInMillis);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapPutCodec::ResponseParameters>(
                        request, key);
            }

            void IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutTransientCodec::RequestParameters::encode(getName(), key, value,
                                                                                         util::getThreadId(),
                                                                                         ttlInMillis);

                invokeOnPartition(request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                                                const serialization::pimpl::Data &value,
                                                                                long ttlInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutIfAbsentCodec::RequestParameters::encode(getName(), key, value,
                                                                                        util::getThreadId(),
                                                                                        ttlInMillis);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapPutIfAbsentCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceIfSameCodec::RequestParameters::encode(getName(), key, oldValue,
                                                                                          newValue,
                                                                                          util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapReplaceIfSameCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::replaceData(const serialization::pimpl::Data &key,
                                                                            const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceCodec::RequestParameters::encode(getName(), key, value,
                                                                                    util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapReplaceCodec::ResponseParameters>(
                        request, key);
            }

            void IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                               long ttl) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapSetCodec::RequestParameters::encode(getName(), key, value,
                                                                                util::getThreadId(), ttl);

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                 -1);

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                 leaseTime);

                invokeOnPartition(request, partitionId);
            }

            bool IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapIsLockedCodec::RequestParameters::encode(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MapIsLockedCodec::ResponseParameters>(request, key);
            }

            bool IMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                    -1, timeInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryLockCodec::ResponseParameters>(request, key);
            }

            void IMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapUnlockCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invokeOnPartition(request, partitionId);
            }

            void IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapForceUnlockCodec::RequestParameters::encode(getName(), key);

                invokeOnPartition(request, partitionId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags), entryEventHandler);
            }

            std::string
            IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, const query::Predicate &predicate,
                                       bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                serialization::pimpl::Data predicateData = toData<serialization::IdentifiedDataSerializable>(predicate);
                return registerListener(createMapEntryListenerCodec(includeValue, predicateData, listenerFlags),
                                        entryEventHandler);
            }

            bool IMapImpl::removeEntryListener(const std::string &registrationId) {
                return context->getClientListenerService().deregisterListener(registrationId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *handler,
                                                   serialization::pimpl::Data &key, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                int32_t listenerFlags = EntryEventType::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags, key), handler);

            }

            std::auto_ptr<map::DataEntryView> IMapImpl::getEntryViewData(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetEntryViewCodec::RequestParameters::encode(getName(), key,
                                                                                         util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<map::DataEntryView>, protocol::codec::MapGetEntryViewCodec::ResponseParameters>(
                        request, key);
            }

            bool IMapImpl::evict(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapEvictCodec::ResponseParameters>(request,
                                                                                                    key);
            }

            void IMapImpl::evictAll() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictAllCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            EntryVector
            IMapImpl::getAllData(const std::map<int, std::vector<serialization::pimpl::Data> > &partitionToKeyData) {
                std::vector<boost::shared_ptr<spi::impl::ClientInvocationFuture> > futures;

                for (std::map<int, std::vector<serialization::pimpl::Data> >::const_iterator it = partitionToKeyData.begin();
                     it != partitionToKeyData.end(); ++it) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapGetAllCodec::RequestParameters::encode(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                EntryVector result;
                // wait for all futures
                BOOST_FOREACH(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &future, futures) {
                                boost::shared_ptr<protocol::ClientMessage> responseForPartition = future->get();
                                protocol::codec::MapGetAllCodec::ResponseParameters resultForPartition =
                                        protocol::codec::MapGetAllCodec::ResponseParameters::decode(
                                                *responseForPartition);
                                result.insert(result.end(), resultForPartition.response.begin(),
                                              resultForPartition.response.end());

                            }

                return result;
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetData(
                    const serialization::IdentifiedDataSerializable &predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetWithPredicateCodec::RequestParameters::encode(getName(),
                                                                                                toData<serialization::IdentifiedDataSerializable>(
                                                                                                        predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySetForPagingPredicateData(
                    const serialization::IdentifiedDataSerializable &predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetWithPagingPredicateCodec::RequestParameters::encode(getName(),
                                                                                                      toData<serialization::IdentifiedDataSerializable>(
                                                                                                              predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetData() {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapEntrySetCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapEntriesWithPredicateCodec::RequestParameters::encode(
                        getName(), toData(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySetForPagingPredicateData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapEntriesWithPagingPredicateCodec::RequestParameters::encode(
                        getName(), toData(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::valuesData() {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> IMapImpl::valuesData(
                    const serialization::IdentifiedDataSerializable &predicate) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesWithPredicateCodec::RequestParameters::encode(
                        getName(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector
            IMapImpl::valuesForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesWithPagingPredicateCodec::RequestParameters::encode(
                        getName(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<EntryVector, protocol::codec::MapValuesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            void IMapImpl::addIndex(const std::string &attribute, bool ordered) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddIndexCodec::RequestParameters::encode(getName(), attribute, ordered);

                invoke(request);
            }

            int IMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapSizeCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<int, protocol::codec::MapSizeCodec::ResponseParameters>(request);
            }

            bool IMapImpl::isEmpty() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapIsEmptyCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<bool, protocol::codec::MapIsEmptyCodec::ResponseParameters>(request);
            }

            void IMapImpl::putAllData(const std::map<int, EntryVector> &partitionedEntries) {
                std::vector<boost::shared_ptr<spi::impl::ClientInvocationFuture> > futures;

                for (std::map<int, EntryVector>::const_iterator it = partitionedEntries.begin();
                     it != partitionedEntries.end(); ++it) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapPutAllCodec::RequestParameters::encode(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                // wait for all futures
                BOOST_FOREACH(const boost::shared_ptr<spi::impl::ClientInvocationFuture> &future, futures) {
                                future->get();
                            }
            }

            void IMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapClearCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            std::auto_ptr<serialization::pimpl::Data> IMapImpl::executeOnKeyData(const serialization::pimpl::Data &key,
                                                                                 const serialization::pimpl::Data &processor) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapExecuteOnKeyCodec::RequestParameters::encode(getName(),
                                                                                         processor,
                                                                                         key,
                                                                                         util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>,
                        protocol::codec::MapExecuteOnKeyCodec::ResponseParameters>(request, partitionId);
            }

            EntryVector IMapImpl::executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                                    const serialization::pimpl::Data &processor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapExecuteOnKeysCodec::RequestParameters::encode(getName(), processor, keys);

                return invokeAndGetResult<EntryVector,
                        protocol::codec::MapExecuteOnKeysCodec::ResponseParameters>(request);
            }

            std::string IMapImpl::addInterceptor(serialization::Portable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::RequestParameters::encode(
                                getName(), toData<serialization::Portable>(interceptor));

                return invokeAndGetResult<std::string, protocol::codec::MapAddInterceptorCodec::ResponseParameters>(
                        request);
            }

            std::string IMapImpl::addInterceptor(serialization::IdentifiedDataSerializable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::RequestParameters::encode(
                                getName(), toData<serialization::IdentifiedDataSerializable>(interceptor));

                return invokeAndGetResult<std::string, protocol::codec::MapAddInterceptorCodec::ResponseParameters>(
                        request);
            }

            void IMapImpl::removeInterceptor(const std::string &id) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveInterceptorCodec::RequestParameters::encode(getName(), id);

                invoke(request);
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &predicate,
                                                  int32_t listenerFlags) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerWithPredicateMessageCodec(getName(), includeValue, listenerFlags,
                                                                      predicate));
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerMessageCodec(getName(), includeValue, listenerFlags));
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags,
                                                  serialization::pimpl::Data &key) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerToKeyCodec(getName(), includeValue, listenerFlags, key));
            }

            IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(const std::string &name,
                                                                                 bool includeValue,
                                                                                 int32_t listenerFlags) : name(name),
                                                                                                          includeValue(
                                                                                                                  includeValue),
                                                                                                          listenerFlags(
                                                                                                                  listenerFlags) {}

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerCodec(name, includeValue, listenerFlags,
                                                                 localOnly).encodeRequest();
            }

            std::string IMapImpl::MapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerCodec(name, includeValue, listenerFlags,
                                                                 false).decodeResponse(responseMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, realRegistrationId).encodeRequest();
            }

            bool IMapImpl::MapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, "").decodeResponse(clientMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec(name, key, includeValue,
                                                                      listenerFlags, localOnly).encodeRequest();
            }

            std::string IMapImpl::MapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec(name, key, includeValue, listenerFlags,
                                                                      false).decodeResponse(responseMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, realRegistrationId).encodeRequest();
            }

            bool IMapImpl::MapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, "").decodeResponse(clientMessage);
            }

            IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(const std::string &name, bool includeValue,
                                                                             int32_t listenerFlags,
                                                                             const serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), listenerFlags(listenerFlags), key(key) {}

            IMapImpl::MapEntryListenerWithPredicateMessageCodec::MapEntryListenerWithPredicateMessageCodec(
                    const std::string &name, bool includeValue, int32_t listenerFlags,
                    serialization::pimpl::Data &predicate) : name(name), includeValue(includeValue),
                                                             listenerFlags(listenerFlags), predicate(predicate) {}

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec(name, predicate, includeValue,
                                                                              listenerFlags, localOnly).encodeRequest();
            }

            std::string IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec(name, predicate, includeValue,
                                                                              listenerFlags, false).decodeResponse(
                        responseMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, realRegistrationId).encodeRequest();
            }

            bool IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec(name, "").decodeResponse(clientMessage);
            }


        }
    }
}

