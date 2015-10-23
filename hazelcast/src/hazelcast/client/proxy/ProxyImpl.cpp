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
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/impl/ClientDestroyRequest.h"
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

            std::string ProxyImpl::listen(const impl::ClientRequest *registrationRequest, int partitionId, impl::BaseEventHandler *handler) {
                return context->getServerListenerService().listen(registrationRequest, partitionId, handler);
            }

            std::string ProxyImpl::listen(const impl::ClientRequest *registrationRequest, impl::BaseEventHandler *handler) {
                return context->getServerListenerService().listen(registrationRequest, handler);
            }

            bool ProxyImpl::stopListening(impl::BaseRemoveListenerRequest *request, const std::string& registrationId) {
                return context->getServerListenerService().stopListening(request, registrationId);
            }

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data& key) {
                return context->getPartitionService().getPartitionId(key);
            }

            serialization::pimpl::Data ProxyImpl::invoke(const impl::ClientRequest *request, int partitionId) {
                spi::InvocationService& invocationService = context->getInvocationService();
                connection::CallFuture future = invocationService.invokeOnPartitionOwner(request, partitionId);
                return future.get();
            }

            connection::CallFuture ProxyImpl::invokeAsync(const impl::ClientRequest *request, int partitionId) {
                spi::InvocationService& invocationService = context->getInvocationService();
                return invocationService.invokeOnPartitionOwner(request, partitionId);
            }

            serialization::pimpl::Data ProxyImpl::invoke(const impl::ClientRequest *request) {
                connection::CallFuture future = context->getInvocationService().invokeOnRandomTarget(request);
                return future.get();
            }

            void ProxyImpl::destroy() {
                onDestroy();
                impl::ClientDestroyRequest *request = new impl::ClientDestroyRequest(DistributedObject::getName(), DistributedObject::getServiceName());
                context->getInvocationService().invokeOnRandomTarget(request);
            }
        }
    }
}

