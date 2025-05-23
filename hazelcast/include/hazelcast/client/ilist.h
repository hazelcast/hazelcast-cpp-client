/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
namespace client {
/**
 * Concurrent, distributed, client implementation of list
 *
 */
class ilist : public proxy::IListImpl
{
    friend class spi::ProxyManager;

public:
    static constexpr const char* SERVICE_NAME = "hz:impl:listService";

    /**
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     *  @param listener that will be added
     *  @param includeValue bool value representing value should be included in
     * ItemEvent or not.
     *  @returns registrationId that can be used to remove item listener
     */
    boost::future<boost::uuids::uuid> add_item_listener(
      item_listener&& listener,
      bool include_value)
    {
        std::unique_ptr<
          impl::item_event_handler<protocol::codec::list_addlistener_handler>>
          itemEventHandler(new impl::item_event_handler<
                           protocol::codec::list_addlistener_handler>(
            get_name(),
            get_context().get_logger(),
            get_context().get_client_cluster_service(),
            get_context().get_serialization_service(),
            std::move(listener),
            include_value));

        return proxy::IListImpl::add_item_listener(std::move(itemEventHandler),
                                                   include_value);
    }

    /**
     *
     * @param element
     * @returns true if list contains element
     */
    template<typename E>
    boost::future<bool> contains(const E& element)
    {
        return proxy::IListImpl::contains(to_data(element));
    }

    /**
     *
     * @returns all elements as std::vector
     */
    template<typename E>
    boost::future<std::vector<E>> to_array()
    {
        return to_object_vector<E>(proxy::IListImpl::to_array_data());
    }

    /**
     *
     * @param element
     * @return true if element is added successfully.
     */
    template<typename E>
    boost::future<bool> add(const E& element)
    {
        return proxy::IListImpl::add(to_data(element));
    }

    /**
     *
     * @param element
     * @return true if element is removed successfully.
     */
    template<typename E>
    boost::future<bool> remove(const E& element)
    {
        return proxy::IListImpl::remove(to_data(element));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if this list contains all elements given in vector.
     */
    template<typename E>
    boost::future<bool> contains_all(const std::vector<E>& elements)
    {
        return proxy::IListImpl::contains_all_data(
          to_data_collection(elements));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if all elements given in vector can be added to list.
     */
    template<typename E>
    boost::future<bool> add_all(const std::vector<E>& elements)
    {
        return proxy::IListImpl::add_all_data(to_data_collection(elements));
    }

    /**
     * Adds elements in vector to the list with given order.
     * Starts adding elements from given index,
     * and shifts others to the right.
     *
     * @param index start point of inserting given elements
     * @param elements vector of elements that will be added to list
     * @return true if list elements are added.
     * @throws index_out_of_bounds if the index is out of range.
     */
    template<typename E>
    boost::future<bool> add_all(int32_t index, const std::vector<E>& elements)
    {
        return proxy::IListImpl::add_all_data(index,
                                              to_data_collection(elements));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if all elements are removed successfully.
     */
    template<typename E>
    boost::future<bool> remove_all(const std::vector<E>& elements)
    {
        return proxy::IListImpl::remove_all_data(to_data_collection(elements));
    }

    /**
     *
     * Removes the elements from this list that are not available in given
     * "elements" vector
     * @param elements std::vector<E>
     * @return true if operation is successful.
     */
    template<typename E>
    boost::future<bool> retain_all(const std::vector<E>& elements)
    {
        return proxy::IListImpl::retain_all_data(to_data_collection(elements));
    }

    /**
     * You can check if element is available by
     *
     *      auto e = list.get(5).get();
     *      if(e.has_value())
     *          //......;
     *
     * @param index
     * @return element in given index. If not available returns empty
     * constructed shared_ptr.
     * @throws index_out_of_bounds if the index is out of range.
     *
     */
    template<typename E>
    boost::future<boost::optional<E>> get(int32_t index)
    {
        return to_object<E>(proxy::IListImpl::get_data(index));
    }

    /**
     * Replaced the element in the given index. And returns element if there
     * were entry before inserting.
     *
     * @param index insert position
     * @param element to be inserted.
     * @return oldElement in given index.
     * @throws Iclass_cast if the type of the specified element is incompatible
     * with the server side.
     * @throws index_out_of_bounds if the index is out of range.
     */
    template<typename E, typename R = E>
    boost::future<boost::optional<R>> set(int32_t index, const E& element)
    {
        return to_object<R>(
          proxy::IListImpl::set_data(index, to_data(element)));
    }

    /**
     * Adds the element to the given index. Shifts others to the right.
     *
     * @param index insert position
     * @param element to be inserted.
     * @throws Iclass_cast if the type of the specified element is incompatible
     * with the server side.
     * @throws index_out_of_bounds if the index is out of range.
     */
    template<typename E>
    boost::future<void> add(int32_t index, const E& element)
    {
        return proxy::IListImpl::add(index, to_data(element));
    }

    /**
     *
     * @param index
     * @return element in given index.  If not available returns empty
     * constructed shared_ptr.
     * @see get
     * @throws index_out_of_bounds if the index is out of range.
     */
    template<typename E>
    boost::future<boost::optional<E>> remove(int32_t index)
    {
        return to_object<E>(proxy::IListImpl::remove_data(index));
    }

    /**
     *
     * @param element that will be searched
     * @return index of first occurrence of given element in the list.
     * Returns -1 if element is not in the list.
     */
    template<typename E>
    boost::future<int> index_of(const E& element)
    {
        return proxy::IListImpl::index_of(to_data(element));
    }

    /**
     * @param element that will be searched
     * @return index of last occurrence of given element in the list.
     * Returns -1 if element is not in the list.
     */
    template<typename E>
    boost::future<int32_t> last_index_of(const E& element)
    {
        return proxy::IListImpl::last_index_of(to_data(element));
    }

    /**
     *
     * @return the sublist as vector between given indexes.
     * @throws index_out_of_bounds if the index is out of range.
     */
    template<typename E>
    boost::future<std::vector<E>> sub_list(int32_t from_index, int32_t to_index)
    {
        return to_object_vector<E>(
          proxy::IListImpl::sub_list_data(from_index, to_index));
    }

private:
    ilist(const std::string& instance_name, spi::ClientContext* context)
      : proxy::IListImpl(instance_name, context)
    {}
};
} // namespace client
} // namespace hazelcast
