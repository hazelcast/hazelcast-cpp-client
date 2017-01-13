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
//
// Created by sancar koyunlu on 30/09/14.
//

#include <limits.h>
#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
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
            MultiMapImpl::MultiMapImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:multiMapService", instanceName, context) {

            }

            bool MultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapPutCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapPutCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::getData(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapGetCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapGetCodec::ResponseParameters>(request, partitionId);
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveEntryCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapRemoveEntryCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::removeData(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapRemoveCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::keySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapKeySetCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapKeySetCodec::ResponseParameters>(request);
            }

            std::vector<serialization::pimpl::Data> MultiMapImpl::valuesData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValuesCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapValuesCodec::ResponseParameters>(request);
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > MultiMapImpl::entrySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapEntrySetCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >, protocol::codec::MultiMapEntrySetCodec::ResponseParameters>(request);
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsKeyCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsKeyCodec::ResponseParameters>(request, partitionId);
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsValueCodec::RequestParameters::encode(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsValueCodec::ResponseParameters>(request);
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsEntryCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MultiMapContainsEntryCodec::ResponseParameters>(request, partitionId);
            }

            int MultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MultiMapSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int, protocol::codec::MultiMapSizeCodec::ResponseParameters>(request);
            }

            void MultiMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapClearCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValueCountCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<int, protocol::codec::MultiMapValueCountCodec::ResponseParameters>(request, partitionId);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec = std::auto_ptr<protocol::codec::IAddListenerCodec>(
                        new protocol::codec::MultiMapAddEntryListenerCodec(getName(), includeValue, false));

                return registerListener(addCodec, entryEventHandler);
            }

            std::string MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                   const serialization::pimpl::Data& key, bool includeValue) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec = std::auto_ptr<protocol::codec::IAddListenerCodec>(
                        new protocol::codec::MultiMapAddEntryListenerToKeyCodec(getName(), key, includeValue, false));

                return registerListener(addCodec, partitionId, entryEventHandler);
            }

            bool MultiMapImpl::removeEntryListener(const std::string& registrationId) {
                protocol::codec::MultiMapRemoveEntryListenerCodec removeCodec(getName(), registrationId);

                return context->getServerListenerService().deRegisterListener(removeCodec);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), -1);

                invoke(request, partitionId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), leaseTime);

                invoke(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapIsLockedCodec::RequestParameters::encode(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MultiMapIsLockedCodec::ResponseParameters>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), LONG_MAX, 0);

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key, long timeInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), LONG_MAX, timeInMillis);

                return invokeAndGetResult<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request, partitionId);
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapUnlockCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapForceUnlockCodec::RequestParameters::encode(getName(), key);

                invoke(request, partitionId);
            }
        }
    }
}

