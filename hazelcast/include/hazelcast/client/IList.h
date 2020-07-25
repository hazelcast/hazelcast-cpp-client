/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/proxy/IListImpl.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        /**
        * Concurrent, distributed, client implementation of list
        *
        */
        class IList : public proxy::IListImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:listService";

            /**
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            *  @param listener that will be added
            *  @param includeValue bool value representing value should be included in ItemEvent or not.
            *  @returns registrationId that can be used to remove item listener
            */
            boost::future<std::string> addItemListener(ItemListener &&listener, bool includeValue) {
                std::unique_ptr<impl::ItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>> itemEventHandler(
                        new impl::ItemEventHandler<protocol::codec::ListAddListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                std::move(listener),
                                includeValue));

                return proxy::IListImpl::addItemListener(std::move(itemEventHandler), includeValue);
            }

            /**
            *
            * @param element
            * @returns true if list contains element
            */
            template<typename E>
            boost::future<bool> contains(const E &element) {
                return proxy::IListImpl::contains(toData(element));
            }

            /**
            *
            * @returns all elements as std::vector
            */
            template<typename E>
            boost::future<std::vector<E>> toArray() {
                return toObjectVector<E>(proxy::IListImpl::toArrayData());
            }

            /**
            *
            * @param element
            * @return true if element is added successfully.
            */
            template<typename E>
            boost::future<bool> add(const E &element) {
                return proxy::IListImpl::add(toData(element));
            }

            /**
            *
            * @param element
            * @return true if element is removed successfully.
            */
            template<typename E>
            boost::future<bool> remove(const E &element) {
                return proxy::IListImpl::remove(toData(element));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this list contains all elements given in vector.
            */
            template<typename E>
            boost::future<bool> containsAll(const std::vector<E> &elements) {
                return proxy::IListImpl::containsAllData(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to list.
            */
            template<typename E>
            boost::future<bool> addAll(const std::vector<E> &elements) {
                return proxy::IListImpl::addAllData(toDataCollection(elements));
            }

            /**
            * Adds elements in vector to the list with given order.
            * Starts adding elements from given index,
            * and shifts others to the right.
            *
            * @param index start point of inserting given elements
            * @param elements vector of elements that will be added to list
            * @return true if list elements are added.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            template<typename E>
            boost::future<bool> addAll(int32_t index, const std::vector<E> &elements) {
                return proxy::IListImpl::addAllData(index, toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            */
            template<typename E>
            boost::future<bool> removeAll(const std::vector<E> &elements) {
                return proxy::IListImpl::removeAllData(toDataCollection(elements));
            }

            /**
            *
            * Removes the elements from this list that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            */
            template<typename E>
            boost::future<bool> retainAll(const std::vector<E> &elements) {
                return proxy::IListImpl::retainAllData(toDataCollection(elements));
            }

            /**
            * You can check if element is available by
            *
            *      auto e = list.get(5).get();
            *      if(e.has_value())
            *          //......;
            *
            * @param index
            * @return element in given index. If not available returns empty constructed shared_ptr.
            * @throws IndexOutOfBoundsException if the index is out of range.
            *
            */
            template<typename E>
            boost::future<boost::optional<E>> get(int32_t index) {
                return toObject<E>(proxy::IListImpl::getData(index));
            }

            /**
            * Replaced the element in the given index. And returns element if there were entry before inserting.
            *
            * @param index insert position
            * @param element to be inserted.
            * @return oldElement in given index.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            template<typename E, typename R = E>
            boost::future<boost::optional<R>> set(int32_t index, const E &element) {
                return toObject<R>(proxy::IListImpl::setData(index, toData(element)));
            }

            /**
            * Adds the element to the given index. Shifts others to the right.
            *
            * @param index insert position
            * @param element to be inserted.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            template<typename E>
            boost::future<void> add(int32_t index, const E &element) {
                return proxy::IListImpl::add(index, toData(element));
            }

            /**
            *
            * @param index
            * @return element in given index.  If not available returns empty constructed shared_ptr.
            * @see get
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            template<typename E>
            boost::future<boost::optional<E>> remove(int32_t index) {
                return toObject<E>(proxy::IListImpl::removeData(index));
            }

            /**
            *
            * @param element that will be searched
            * @return index of first occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            */
            template<typename E>
            boost::future<int> indexOf(const E &element) {
                return proxy::IListImpl::indexOf(toData(element));
            }

            /**
            * @param element that will be searched
            * @return index of last occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            */
            template<typename E>
            boost::future<int32_t> lastIndexOf(const E &element) {
                return proxy::IListImpl::lastIndexOf(toData(element));
            }

            /**
            *
            * @return the sublist as vector between given indexes.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            template<typename E>
            boost::future<std::vector<E>> subList(int32_t fromIndex, int32_t toIndex) {
                return toObjectVector<E>(proxy::IListImpl::subListData(fromIndex, toIndex));
            }

        private:
            IList(const std::string &instanceName, spi::ClientContext *context) : proxy::IListImpl(instanceName,
                                                                                                   context) {}
        };
    }
}

