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
#include "hazelcast/client/collection/CollectionAddListenerRequest.h"
#include "hazelcast/client/collection/CollectionRemoveListenerRequest.h"
#include "hazelcast/client/collection/ListAddRequest.h"
#include "hazelcast/client/collection/ListRemoveRequest.h"
#include "hazelcast/client/collection/ListAddAllRequest.h"
#include "hazelcast/client/collection/ListGetRequest.h"
#include "hazelcast/client/collection/ListSetRequest.h"
#include "hazelcast/client/collection/ListIndexOfRequest.h"
#include "hazelcast/client/collection/ListSubRequest.h"
#include "hazelcast/client/collection/CollectionSizeRequest.h"
#include "hazelcast/client/collection/CollectionContainsRequest.h"
#include "hazelcast/client/collection/CollectionGetAllRequest.h"
#include "hazelcast/client/collection/CollectionCompareAndRemoveRequest.h"
#include "hazelcast/client/collection/CollectionRemoveRequest.h"
#include "hazelcast/client/collection/CollectionClearRequest.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/impl/SerializableCollection.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            IListImpl::IListImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::string IListImpl::addItemListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                collection::CollectionAddListenerRequest *request = new collection::CollectionAddListenerRequest(getName(), getServiceName(), includeValue);
                return listen(request, entryEventHandler);
            }

            bool IListImpl::removeItemListener(const std::string& registrationId) {
                collection::CollectionRemoveListenerRequest *request = new collection::CollectionRemoveListenerRequest(getName(), getServiceName(), registrationId);
                return stopListening(request, registrationId);
            }

            int IListImpl::size() {
                collection::CollectionSizeRequest *request = new collection::CollectionSizeRequest(getName(), getServiceName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }

            bool IListImpl::contains(const serialization::pimpl::Data& element) {
                std::vector<serialization::pimpl::Data> valueSet;
                valueSet.push_back(element);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), valueSet);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            std::vector<serialization::pimpl::Data> IListImpl::toArray() {
                collection::CollectionGetAllRequest *request = new collection::CollectionGetAllRequest(getName(), getServiceName());
                serialization::pimpl::Data data = invoke(request, partitionId);
                boost::shared_ptr<impl::SerializableCollection> result = context->getSerializationService().toObject<impl::SerializableCollection>(data);
                return result->getCollection();
            }

            bool IListImpl::add(const serialization::pimpl::Data& element) {
                collection::CollectionAddRequest *request = new collection::CollectionAddRequest(getName(), getServiceName(), element);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IListImpl::remove(const serialization::pimpl::Data& element) {
                collection::CollectionRemoveRequest *request = new collection::CollectionRemoveRequest(getName(), getServiceName(), element);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IListImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IListImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionAddAllRequest *request = new collection::CollectionAddAllRequest(getName(), getServiceName(), elements);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IListImpl::addAll(int index, const std::vector<serialization::pimpl::Data>& elements) {
                list::ListAddAllRequest *request = new list::ListAddAllRequest(getName(), getServiceName(), elements, index);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;;
            }

            bool IListImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), elements, false);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            bool IListImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), elements, true);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, bool)
                return *result;
            }

            void IListImpl::clear() {
                collection::CollectionClearRequest *request = new collection::CollectionClearRequest(getName(), getServiceName());
                invoke(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::get(int index) {
                list::ListGetRequest *request = new list::ListGetRequest(getName(), getServiceName(), index);
                return invoke(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::set(int index, const serialization::pimpl::Data& element) {

                list::ListSetRequest *request = new list::ListSetRequest(getName(), getServiceName(), element, index);
                return invoke(request, partitionId);
            }

            void IListImpl::add(int index, const serialization::pimpl::Data& element) {

                list::ListAddRequest *request = new list::ListAddRequest(getName(), getServiceName(), element, index);
                invoke(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::remove(int index) {
                list::ListRemoveRequest *request = new list::ListRemoveRequest(getName(), getServiceName(), index);
                return invoke(request, partitionId);
            }

            int IListImpl::indexOf(const serialization::pimpl::Data& element) {
                list::ListIndexOfRequest *request = new list::ListIndexOfRequest(getName(), getServiceName(), element, false);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }

            int IListImpl::lastIndexOf(const serialization::pimpl::Data& element) {
                list::ListIndexOfRequest *request = new list::ListIndexOfRequest(getName(), getServiceName(), element, true);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, int)
                return *result;
            }

            std::vector<serialization::pimpl::Data> IListImpl::subList(int fromIndex, int toIndex) {
                list::ListSubRequest *request = new list::ListSubRequest(getName(), getServiceName(), fromIndex, toIndex);
                serialization::pimpl::Data data = invoke(request, partitionId);
                DESERIALIZE(data, impl::SerializableCollection)
                return result->getCollection();
            }

        }
    }
}

