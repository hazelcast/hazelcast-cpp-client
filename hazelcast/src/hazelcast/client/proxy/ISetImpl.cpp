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

#include "hazelcast/client/proxy/ISetImpl.h"

#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ClientListenerService.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/SetSizeCodec.h"
#include "hazelcast/client/protocol/codec/SetContainsCodec.h"
#include "hazelcast/client/protocol/codec/SetContainsAllCodec.h"
#include "hazelcast/client/protocol/codec/SetAddCodec.h"
#include "hazelcast/client/protocol/codec/SetRemoveCodec.h"
#include "hazelcast/client/protocol/codec/SetAddAllCodec.h"
#include "hazelcast/client/protocol/codec/SetCompareAndRemoveAllCodec.h"
#include "hazelcast/client/protocol/codec/SetCompareAndRetainAllCodec.h"
#include "hazelcast/client/protocol/codec/SetClearCodec.h"
#include "hazelcast/client/protocol/codec/SetGetAllCodec.h"
#include "hazelcast/client/protocol/codec/SetAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/SetRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/SetIsEmptyCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ISetImpl::ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext)
            : ProxyImpl("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string ISetImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            bool ISetImpl::removeItemListener(const std::string& registrationId) {
                return context->getClientListenerService().deregisterListener(registrationId);
            }

            int ISetImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::SetSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int, protocol::codec::SetSizeCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::isEmpty() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::SetIsEmptyCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<bool, protocol::codec::SetIsEmptyCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetContainsCodec::ResponseParameters>(request, partitionId);
            }

            std::vector<serialization::pimpl::Data>  ISetImpl::toArrayData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetGetAllCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::SetGetAllCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::add(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetAddCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetRemoveCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::SetRemoveCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetContainsAllCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetAddAllCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRemoveAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetCompareAndRemoveAllCodec::ResponseParameters>(request, partitionId);
            }

            bool ISetImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRetainAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::SetCompareAndRetainAllCodec::ResponseParameters>(request, partitionId);
            }

            void ISetImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetClearCodec::RequestParameters::encode(getName());

                invokeOnPartition(request, partitionId);
            }

            boost::shared_ptr<spi::impl::ListenerMessageCodec>
            ISetImpl::createItemListenerCodec(bool includeValue) {
                return boost::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new SetListenerMessageCodec(getName(), includeValue));
            }

            ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(const std::string &name,
                                                                             bool includeValue) : name(name),
                                                                                                  includeValue(
                                                                                                          includeValue) {}

            std::auto_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::SetAddListenerCodec(name, includeValue, localOnly).encodeRequest();
            }

            std::string ISetImpl::SetListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::SetAddListenerCodec(name, includeValue, false).decodeResponse(responseMessage);
            }

            std::auto_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::SetRemoveListenerCodec(name, realRegistrationId).encodeRequest();
            }

            bool ISetImpl::SetListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::SetRemoveListenerCodec(name, "").decodeResponse(clientMessage);
            }

        }
    }
}
