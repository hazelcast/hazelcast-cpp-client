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

#include "hazelcast/client/spi/ServerListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            IQueueImpl::IQueueImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(data);
            }

            std::string IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec = std::auto_ptr<protocol::codec::IAddListenerCodec>(
                        new protocol::codec::QueueAddListenerCodec(getName(), includeValue, false));

                return registerListener(addCodec, itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string &registrationId) {
                protocol::codec::QueueRemoveListenerCodec removeCodec(getName(), registrationId);

                return context->getServerListenerService().deRegisterListener(removeCodec);
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data &element, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueOfferCodec::RequestParameters::encode(getName(), element,
                                                                                    timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::QueueOfferCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            void IQueueImpl::put(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePutCodec::RequestParameters::encode(getName(), element);

                invoke(request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IQueueImpl::pollData(long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePollCodec::RequestParameters::encode(getName(), timeoutInMillis);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::QueuePollCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemainingCapacityCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int, protocol::codec::QueueRemainingCapacityCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemoveCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueRemoveCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::drainToData(size_t maxElements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToMaxSizeCodec::RequestParameters::encode(getName(), maxElements);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }


            std::auto_ptr<serialization::pimpl::Data> IQueueImpl::peekData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePeekCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::QueuePeekCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int, protocol::codec::QueueSizeCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            std::vector<serialization::pimpl::Data> IQueueImpl::toArrayData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIteratorCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::QueueIteratorCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueContainsAllCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueAddAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueAddAllCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRemoveAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRetainAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::QueueCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IQueueImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueClearCodec::RequestParameters::encode(getName());

                invoke(request, partitionId);
            }

        }
    }
}

