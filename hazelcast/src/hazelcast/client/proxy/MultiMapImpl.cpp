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
// Created by sancar koyunlu on 30/09/14.
//

#include <limits.h>
#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/spi/ClientListenerService.h"
#include "hazelcast/util/Util.h"

// Includes for codec classes
#include "hazelcast/client/protocol/codec/MultiMapPutCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapGetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapKeySetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapValuesCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapEntrySetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsKeyCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsValueCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsEntryCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapSizeCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapClearCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapAddEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapLockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapTryLockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapIsLockedCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapForceUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveEntryCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapValueCountCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:multiMapService", instanceName, context) {
                // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
                lockReferenceIdGenerator = context->getLockReferenceIdGenerator();
            }

            bool MultiMapImpl::put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapPutCodec::encodeRequest(getName(), key, value,
                                                                         util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapPutCodec::ResponseParameters>(request, key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::getData(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapGetCodec::encodeRequest(getName(), key,
                                                                         util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapGetCodec::ResponseParameters>(
                        request, key);
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveEntryCodec::encodeRequest(getName(), key, value,
                                                                                 util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapRemoveEntryCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::removeData(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveCodec::encodeRequest(getName(), key,
                                                                            util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::keySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapKeySetCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapKeySetCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::valuesData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValuesCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapValuesCodec::ResponseParameters>(
                        request);
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >
            MultiMapImpl::entrySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapEntrySetCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >, protocol::codec::MultiMapEntrySetCodec::ResponseParameters>(
                        request);
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsKeyCodec::encodeRequest(getName(), key,
                                                                                 util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsKeyCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsValueCodec::encodeRequest(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsValueCodec::ResponseParameters>(
                        request);
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsEntryCodec::encodeRequest(getName(), key, value,
                                                                                   util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsEntryCodec::ResponseParameters>(
                        request, key);
            }

            int MultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MultiMapSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetResult<int, protocol::codec::MultiMapSizeCodec::ResponseParameters>(request);
            }

            void MultiMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapClearCodec::encodeRequest(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValueCountCodec::encodeRequest(getName(), key,
                                                                                util::getThreadId());

                return invokeAndGetResult<int, protocol::codec::MultiMapValueCountCodec::ResponseParameters>(request,
                                                                                                             key);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue), entryEventHandler);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                       serialization::pimpl::Data &key, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue, key), entryEventHandler);
            }

            bool MultiMapImpl::removeEntryListener(const std::string &registrationId) {
                return context->getClientListenerService().deregisterListener(registrationId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data &key) {
                lock(key, -1);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::encodeRequest(getName(), key, util::getThreadId(),
                                                                          leaseTime,
                                                                          lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapIsLockedCodec::encodeRequest(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MultiMapIsLockedCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key,
                                                                             util::getThreadId(), LONG_MAX,
                                                                             0,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key, util::getThreadId(),
                                                                             LONG_MAX, timeInMillis,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapUnlockCodec::encodeRequest(getName(), key, util::getThreadId(),
                                                                            lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapForceUnlockCodec::encodeRequest(getName(), key,
                                                                                 lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerMessageCodec(getName(), includeValue));
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &key) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerToKeyCodec(getName(), includeValue, key));
            }

            void MultiMapImpl::onInitialize() {
                ProxyImpl::onInitialize();

                lockReferenceIdGenerator = context->getLockReferenceIdGenerator();
            }

            MultiMapImpl::MultiMapEntryListenerMessageCodec::MultiMapEntryListenerMessageCodec(const std::string &name,
                                                                                               bool includeValue)
                    : name(name),
                      includeValue(
                              includeValue) {
            }

            std::auto_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::auto_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            std::auto_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::encodeRequest(name, key, includeValue,
                                                                                          localOnly);
            }

            std::string MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::auto_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(const std::string &name,
                                                                                           bool includeValue,
                                                                                           serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), key(key) {}


        }
    }
}

