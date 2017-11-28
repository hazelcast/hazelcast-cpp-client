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
// Created by sancar koyunlu on 01/10/14.
//

#include <boost/foreach.hpp>

#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/protocol/codec/IAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/IRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/ClientDestroyProxyCodec.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/CallFuture.h"


namespace hazelcast {
    namespace client {
        namespace proxy {

            ProxyImpl::ProxyImpl(const std::string& serviceName, const std::string& objectName, spi::ClientContext *context)
            : DistributedObject(serviceName, objectName)
            , context(context) {

            }

            ProxyImpl::~ProxyImpl() {

            }

            std::string ProxyImpl::registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                                                    int partitionId, impl::BaseEventHandler *handler) {
                return context->getServerListenerService().registerListener(addListenerCodec, partitionId, handler);
            }

            std::string ProxyImpl::registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                                                    impl::BaseEventHandler *handler) {
                return context->getServerListenerService().registerListener(addListenerCodec, handler);
            }

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data& key) {
                return context->getPartitionService().getPartitionId(key);
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                spi::InvocationService& invocationService = context->getInvocationService();
                connection::CallFuture future = invocationService.invokeOnPartitionOwner(request, partitionId);
                return future.get();
            }

            connection::CallFuture ProxyImpl::invokeAndGetFuture(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                spi::InvocationService& invocationService = context->getInvocationService();
                return invocationService.invokeOnPartitionOwner(request, partitionId);
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request) {
                connection::CallFuture future = context->getInvocationService().invokeOnRandomTarget(request);
                return future.get();
            }

            void ProxyImpl::destroy() {
                onDestroy();

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::ClientDestroyProxyCodec::RequestParameters::encode(
                                        DistributedObject::getName(), DistributedObject::getServiceName());

                context->getInvocationService().invokeOnRandomTarget(request).get();
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                                     boost::shared_ptr<connection::Connection> conn) {
                connection::CallFuture future = context->getInvocationService().invokeOnConnection(request, conn);
                return future.get();
            }

            std::vector<hazelcast::client::TypedData>
            ProxyImpl::toTypedDataCollection(const std::vector<serialization::pimpl::Data> &values) const {
                std::vector<hazelcast::client::TypedData> result;
                typedef std::vector<serialization::pimpl::Data> VALUES;
                BOOST_FOREACH(const VALUES::value_type &value , values) {
                    result.push_back(TypedData(
                            std::auto_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(value)),
                            context->getSerializationService()));
                }
                return result;
            }

            std::vector<std::pair<TypedData, TypedData> > ProxyImpl::toTypedDataEntrySet(
                    const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &dataEntrySet) {
                typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > ENTRIES_DATA;
                std::vector<std::pair<TypedData, TypedData> > result;
                BOOST_FOREACH(const ENTRIES_DATA::value_type &value , dataEntrySet) {
                                serialization::pimpl::SerializationService &serializationService = context->getSerializationService();
                                TypedData keyData(std::auto_ptr<serialization::pimpl::Data>(
                                        new serialization::pimpl::Data(value.first)), serializationService);
                                TypedData valueData(std::auto_ptr<serialization::pimpl::Data>(
                                        new serialization::pimpl::Data(value.second)), serializationService);
                                result.push_back(std::make_pair(keyData, valueData));
                            }
                return result;
            }

            boost::shared_ptr<serialization::pimpl::Data> ProxyImpl::toShared(const serialization::pimpl::Data &data) {
                return boost::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
            }
        }
    }
}

