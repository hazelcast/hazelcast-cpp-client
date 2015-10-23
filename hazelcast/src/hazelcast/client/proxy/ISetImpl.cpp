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

#include "hazelcast/client/proxy/ISetImpl.h"
#include "hazelcast/client/collection/CollectionAddListenerRequest.h"
#include "hazelcast/client/collection/CollectionRemoveListenerRequest.h"
#include "hazelcast/client/collection/CollectionSizeRequest.h"
#include "hazelcast/client/collection/CollectionContainsRequest.h"
#include "hazelcast/client/collection/CollectionRemoveRequest.h"
#include "hazelcast/client/collection/CollectionAddAllRequest.h"
#include "hazelcast/client/collection/CollectionCompareAndRemoveRequest.h"
#include "hazelcast/client/collection/CollectionGetAllRequest.h"
#include "hazelcast/client/collection/CollectionAddRequest.h"
#include "hazelcast/client/collection/CollectionClearRequest.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/SerializableCollection.h"


namespace hazelcast {
    namespace client {
        namespace proxy {
            ISetImpl::ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext)
            : ProxyImpl("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string ISetImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                collection::CollectionAddListenerRequest *request = new collection::CollectionAddListenerRequest(getName(), getServiceName(), includeValue);
                return listen(request, itemEventHandler);
            }

            bool ISetImpl::removeItemListener(const std::string& registrationId) {
                collection::CollectionRemoveListenerRequest *request = new collection::CollectionRemoveListenerRequest(getName(), getServiceName(), registrationId);
                return stopListening(request, registrationId);
            }

            int ISetImpl::size() {
                collection::CollectionSizeRequest *request = new collection::CollectionSizeRequest(getName(), getServiceName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }


            bool ISetImpl::contains(const serialization::pimpl::Data& element) {
                std::vector<serialization::pimpl::Data> valueSet;
                valueSet.push_back(element);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), valueSet);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            std::vector<serialization::pimpl::Data> ISetImpl::toArray() {
                collection::CollectionGetAllRequest *request = new collection::CollectionGetAllRequest(getName(), getServiceName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::SerializableCollection)
                return result->getCollection();
            }

            bool ISetImpl::add(const serialization::pimpl::Data& element) {
                collection::CollectionAddRequest *request = new collection::CollectionAddRequest(getName(), getServiceName(), element);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool ISetImpl::remove(const serialization::pimpl::Data& element) {
                collection::CollectionRemoveRequest *request = new collection::CollectionRemoveRequest(getName(), getServiceName(), element);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool ISetImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool ISetImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionAddAllRequest *request = new collection::CollectionAddAllRequest(getName(), getServiceName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool ISetImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), elements, false);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool ISetImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), elements, true);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            void ISetImpl::clear() {
                collection::CollectionClearRequest *request = new collection::CollectionClearRequest(getName(), getServiceName());
                invoke(request, partitionId);
            }
        }
    }
}
