/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/IListImpl.h"


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/ListSizeCodec.h"
#include "hazelcast/client/protocol/codec/ListContainsCodec.h"
#include "hazelcast/client/protocol/codec/ListContainsAllCodec.h"
#include "hazelcast/client/protocol/codec/ListAddCodec.h"
#include "hazelcast/client/protocol/codec/ListRemoveCodec.h"
#include "hazelcast/client/protocol/codec/ListAddAllCodec.h"
#include "hazelcast/client/protocol/codec/ListCompareAndRemoveAllCodec.h"
#include "hazelcast/client/protocol/codec/ListCompareAndRetainAllCodec.h"
#include "hazelcast/client/protocol/codec/ListClearCodec.h"
#include "hazelcast/client/protocol/codec/ListGetAllCodec.h"
#include "hazelcast/client/protocol/codec/ListAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/ListRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/ListIsEmptyCodec.h"
#include "hazelcast/client/protocol/codec/ListAddAllWithIndexCodec.h"
#include "hazelcast/client/protocol/codec/ListGetCodec.h"
#include "hazelcast/client/protocol/codec/ListSetCodec.h"
#include "hazelcast/client/protocol/codec/ListAddWithIndexCodec.h"
#include "hazelcast/client/protocol/codec/ListRemoveWithIndexCodec.h"
#include "hazelcast/client/protocol/codec/ListLastIndexOfCodec.h"
#include "hazelcast/client/protocol/codec/ListIndexOfCodec.h"
#include "hazelcast/client/protocol/codec/ListSubCodec.h"
#include "hazelcast/client/protocol/codec/ListIteratorCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            IListImpl::IListImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string IListImpl::addItemListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec = std::auto_ptr<protocol::codec::IAddListenerCodec>(
                        new protocol::codec::ListAddListenerCodec(getName(), includeValue,
                                                                  false));

                return registerListener(addCodec, entryEventHandler);
            }

            bool IListImpl::removeItemListener(const std::string &registrationId) {
                protocol::codec::ListRemoveListenerCodec removeCodec(getName(), registrationId);

                return context->getServerListenerService().deRegisterListener(removeCodec);
            }

            int IListImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int, protocol::codec::ListSizeCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            bool IListImpl::contains(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListContainsCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListContainsCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            std::vector<serialization::pimpl::Data> IListImpl::toArrayData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListGetAllCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::ListGetAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IListImpl::add(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListAddCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            bool IListImpl::remove(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListRemoveCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool, protocol::codec::ListRemoveCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            bool IListImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListContainsAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListContainsAllCodec::ResponseParameters>(request,
                                                                                                           partitionId);
            }

            bool IListImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListAddAllCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            bool IListImpl::addAll(int index, const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddAllWithIndexCodec::RequestParameters::encode(getName(), index,
                                                                                             elements);

                return invokeAndGetResult<bool, protocol::codec::ListAddAllWithIndexCodec::ResponseParameters>(request,
                                                                                                               partitionId);
            }

            bool IListImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListCompareAndRemoveAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IListImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListCompareAndRetainAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool, protocol::codec::ListCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IListImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListClearCodec::RequestParameters::encode(getName());

                invoke(request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IListImpl::getData(int index) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListGetCodec::RequestParameters::encode(getName(), index);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::ListGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IListImpl::setData(int index,
                                                                     const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSetCodec::RequestParameters::encode(getName(), index, element);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::ListSetCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IListImpl::add(int index, const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListAddWithIndexCodec::RequestParameters::encode(getName(), index, element);

                invoke(request, partitionId);
            }

            std::auto_ptr<serialization::pimpl::Data> IListImpl::removeData(int index) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListRemoveWithIndexCodec::RequestParameters::encode(getName(), index);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::ListRemoveWithIndexCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IListImpl::indexOf(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListIndexOfCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<int, protocol::codec::ListIndexOfCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            int IListImpl::lastIndexOf(const serialization::pimpl::Data &element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListLastIndexOfCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<int, protocol::codec::ListLastIndexOfCodec::ResponseParameters>(request,
                                                                                                          partitionId);
            }

            std::vector<serialization::pimpl::Data> IListImpl::subListData(int fromIndex, int toIndex) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::ListSubCodec::RequestParameters::encode(getName(), fromIndex, toIndex);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::ListSubCodec::ResponseParameters>(
                        request, partitionId);
            }

        }
    }
}

