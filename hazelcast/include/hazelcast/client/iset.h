/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/ISetImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
namespace client {
/**
 * Concurrent, distributed client implementation of std::unordered_set.
 *
 */
class HAZELCAST_API iset : public proxy::ISetImpl
{
    friend class spi::ProxyManager;

public:
    static constexpr const char* SERVICE_NAME = "hz:impl:setService";

    /**
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     *  @param listener to be added
     *  @param includeValue boolean value representing value should be included
     * in incoming ItemEvent or not.
     *  @returns registrationId that can be used to remove item listener
     */
    boost::future<boost::uuids::uuid> add_item_listener(
      item_listener&& listener,
      bool include_value)
    {
        std::unique_ptr<
          impl::item_event_handler<protocol::codec::set_addlistener_handler>>
          itemEventHandler(new impl::item_event_handler<
                           protocol::codec::set_addlistener_handler>(
            get_name(),
            get_context().get_client_cluster_service(),
            get_context().get_serialization_service(),
            std::move(listener),
            include_value));

        return proxy::ISetImpl::add_item_listener(std::move(itemEventHandler),
                                                  include_value);
    }

    /**
     *
     * @param element to be searched
     * @returns true if set contains element
     */
    template<typename E>
    boost::future<bool> contains(const E& element)
    {
        return proxy::ISetImpl::contains(to_data(element));
    }

    /**
     *
     * @returns all elements as std::vector
     */
    template<typename E>
    boost::future<std::vector<E>> to_array()
    {
        return to_object_vector<E>(proxy::ISetImpl::to_array_data());
    }

    /**
     *
     * @param element to be added
     * @return true if element is added successfully. If elements was already
     * there returns false.
     */
    template<typename E>
    boost::future<bool> add(const E& element)
    {
        return proxy::ISetImpl::add(to_data(element));
    }

    /**
     *
     * @param element to be removed
     * @return true if element is removed successfully.
     */
    template<typename E>
    boost::future<bool> remove(const E& element)
    {
        return proxy::ISetImpl::remove(to_data(element));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if this set contains all elements given in vector.
     */
    template<typename E>
    boost::future<bool> contains_all(const std::vector<E>& elements)
    {
        return proxy::ISetImpl::contains_all(to_data_collection(elements));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if all elements given in vector can be added to set.
     */
    template<typename E>
    boost::future<bool> add_all(const std::vector<E>& elements)
    {
        std::vector<serialization::pimpl::data> dataCollection =
          to_data_collection(elements);
        return proxy::ISetImpl::add_all(to_data_collection(elements));
    }

    /**
     *
     * @param elements std::vector<E>
     * @return true if all elements are removed successfully.
     */
    template<typename E>
    boost::future<bool> remove_all(const std::vector<E>& elements)
    {
        return proxy::ISetImpl::remove_all(to_data_collection(elements));
    }

    /**
     *
     * Removes the elements from this set that are not available in given
     * "elements" vector
     * @param elements std::vector<E>
     * @return true if operation is successful.
     */
    template<typename E>
    boost::future<bool> retain_all(const std::vector<E>& elements)
    {
        return proxy::ISetImpl::retain_all(to_data_collection(elements));
    }

private:
    iset(const std::string& instance_name, spi::ClientContext* context)
      : proxy::ISetImpl(instance_name, context)
    {}
};
} // namespace client
} // namespace hazelcast
