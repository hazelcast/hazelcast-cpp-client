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
//
// Created by sancar koyunlu on 30/09/14.
//

#include "hazelcast/client/proxy/IQueueImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/QueueOfferCodec.h"
#include "hazelcast/client/protocol/codec/QueuePutCodec.h"
#include "hazelcast/client/protocol/codec/QueueSizeCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemoveCodec.h"
#include "hazelcast/client/protocol/codec/QueuePollCodec.h"
#include "hazelcast/client/protocol/codec/QueuePeekCodec.h"
#include "hazelcast/client/protocol/codec/QueueIteratorCodec.h"
#include "hazelcast/client/protocol/codec/QueueDrainToCodec.h"
#include "hazelcast/client/protocol/codec/QueueDrainToMaxSizeCodec.h"
#include "hazelcast/client/protocol/codec/QueueContainsCodec.h"
#include "hazelcast/client/protocol/codec/QueueContainsAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueCompareAndRemoveAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueCompareAndRetainAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueClearCodec.h"
#include "hazelcast/client/protocol/codec/QueueAddAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemainingCapacityCodec.h"
#include "hazelcast/client/protocol/codec/QueueIsEmptyCodec.h"

#include "hazelcast/client/spi/ClientListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            IQueueImpl::IQueueImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = getContext().getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(data);
            }

            std::string IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data &element, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueOfferCodec::encodeRequest(getName(), element,
                                                                                    timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::QueueOfferCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            void IQueueImpl::put(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePutCodec::encodeRequest(getName(), element);

                invokeOnPartition(request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IQueueImpl::pollData(long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePollCodec::encodeRequest(getName(), timeoutInMillis);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::QueuePollCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemainingCapacityCodec::encodeRequest(getName());

                return invokeAndGetResult<int, protocol::codec::QueueRemainingCapacityCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueRemoveCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainToData(size_t maxElements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToMaxSizeCodec::encodeRequest(getName(), (int32_t) maxElements);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainToData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IQueueImpl::peekData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePeekCodec::encodeRequest(getName());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::QueuePeekCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueSizeCodec::encodeRequest(getName());

                return invokeAndGetResult<int, protocol::codec::QueueSizeCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            bool IQueueImpl::isEmpty() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIsEmptyCodec::encodeRequest(getName());

                return invokeAndGetResult<bool, protocol::codec::QueueIsEmptyCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::toArrayData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIteratorCodec::encodeRequest(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueIteratorCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsAllCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueAddAllCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IQueueImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            IQueueImpl::createItemListenerCodec(bool includeValue) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new QueueListenerMessageCodec(getName(), includeValue));
            }

            IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(const std::string &name,
                                                                          bool includeValue) : name(name),
                                                                                               includeValue(
                                                                                                       includeValue) {}

            std::auto_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::QueueAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string IQueueImpl::QueueListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::QueueAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::auto_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::QueueRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IQueueImpl::QueueListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::QueueRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

        }
    }
}

