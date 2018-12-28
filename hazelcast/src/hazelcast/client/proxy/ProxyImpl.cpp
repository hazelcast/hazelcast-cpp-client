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
// Created by sancar koyunlu on 01/10/14.
//

#include <boost/foreach.hpp>

#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/protocol/codec/ClientDestroyProxyCodec.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientPartitionService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/ClientInvocationService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ProxyImpl::ProxyImpl(const std::string &serviceName, const std::string &objectName,
                                 spi::ClientContext *context)
                    : ClientProxy(objectName, serviceName, *context) {
            }

            ProxyImpl::~ProxyImpl() {
            }

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data &key) {
                return getContext().getPartitionService().getPartitionId(key);
            }

            boost::shared_ptr<protocol::ClientMessage> ProxyImpl::invokeOnPartition(
                    std::auto_ptr<protocol::ClientMessage> request, int partitionId) {

                boost::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return invocation->invoke()->get();
            }

            boost::shared_ptr<spi::impl::ClientInvocationFuture>
            ProxyImpl::invokeAndGetFuture(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                boost::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), partitionId);
                return invocation->invoke();
            }

            boost::shared_ptr<protocol::ClientMessage>
            ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request) {
                boost::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName());
                return invocation->invoke()->get();
            }

            boost::shared_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                                         boost::shared_ptr<connection::Connection> conn) {
                boost::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), conn);
                return invocation->invoke()->get();
            }

            boost::shared_ptr<protocol::ClientMessage>
            ProxyImpl::invokeOnAddress(std::auto_ptr<protocol::ClientMessage> request, const Address &address) {
                boost::shared_ptr<spi::impl::ClientInvocation> invocation = spi::impl::ClientInvocation::create(
                        getContext(), request, getName(), address);
                return invocation->invoke()->get();
            }

            std::vector<hazelcast::client::TypedData>
            ProxyImpl::toTypedDataCollection(const std::vector<serialization::pimpl::Data> &values) {
                std::vector<hazelcast::client::TypedData> result;
                typedef std::vector<serialization::pimpl::Data> VALUES;
                BOOST_FOREACH(const VALUES::value_type &value, values) {
                                result.push_back(TypedData(
                                        std::auto_ptr<serialization::pimpl::Data>(
                                                new serialization::pimpl::Data(value)),
                                        getContext().getSerializationService()));
                            }
                return result;
            }

            std::vector<std::pair<TypedData, TypedData> > ProxyImpl::toTypedDataEntrySet(
                    const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &dataEntrySet) {
                typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > ENTRIES_DATA;
                std::vector<std::pair<TypedData, TypedData> > result;
                BOOST_FOREACH(const ENTRIES_DATA::value_type &value, dataEntrySet) {
                                serialization::pimpl::SerializationService &serializationService = getContext().getSerializationService();
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

