/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/item_listener.h"
#include "hazelcast/client/proxy/IQueueImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
    namespace client {
        /**
        * Concurrent, blocking, distributed, observable, client queue.
        *
        */
        class iqueue : public proxy::IQueueImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:queueService";

            /**
            * Adds an item listener for this collection. Listener will get notified
            * for all collection add/remove events.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener     item listener
            * @param includeValue <tt>true</tt> updated item should be passed
            *                     to the item listener, <tt>false</tt> otherwise.
            * @return returns registration id.
            */
            boost::future<boost::uuids::uuid> add_item_listener(item_listener &&listener, bool include_value) {
                std::unique_ptr<impl::item_event_handler<protocol::codec::queue_addlistener_handler>> itemEventHandler(
                        new impl::item_event_handler<protocol::codec::queue_addlistener_handler>(
                                get_name(), get_context().get_client_cluster_service(),
                                get_context().get_serialization_service(),
                                std::move(listener),
                                include_value));

                return proxy::IQueueImpl::add_item_listener(std::move(itemEventHandler), include_value);
            }

            /**
            * Inserts the specified element into this queue.
            *
            * @param element to add
            * @return <tt>true</tt> if the element was added to this queue, else
            *         <tt>false</tt>
            */
            template<typename E>
            boost::future<bool> offer(const E &element) {
                return offer(element, std::chrono::milliseconds(0));
            }

            /**
            * Puts the element into queue.
            * If queue is  full waits for space to became available.
            */
            template<typename E>
            boost::future<void> put(const E &element) {
                return proxy::IQueueImpl::put(to_data(element));
            }

            /**
            * Inserts the specified element into this queue.
            * If queue is  full waits for space to became available for specified time.
            *
            * @param element to add
            * @param timeout how long to wait before giving up
            * @return <tt>true</tt> if successful, or <tt>false</tt> if
            *         the specified waiting time elapses before space is available
            */
            template<typename E>
            boost::future<bool> offer(const E &element, std::chrono::milliseconds timeout) {
                return proxy::IQueueImpl::offer(to_data(element), timeout);
            }

            /**
            *
            * @return the head of the queue. If queue is empty waits for an item to be added.
            */
            template<typename E>
            boost::future<boost::optional<E>> take() {
                return to_object<E>(proxy::IQueueImpl::take_data());
            }

            /**
            *
            * @param timeout time to wait if item is not available.
            * @return the head of the queue. If queue is empty waits for specified time.
            */
            template<typename E>
            boost::future<boost::optional<E>> poll(std::chrono::milliseconds timeout) {
                return to_object<E>(proxy::IQueueImpl::poll_data(timeout));
            }

            /**
            *
            * @param element to be removed.
            * @return true if element removed successfully.
            */
            template<typename E>
            boost::future<bool> remove(const E &element) {
                return proxy::IQueueImpl::remove(to_data(element));
            }

            /**
            *
            * @param element to be checked.
            * @return true if queue contains the element.
            */
            template<typename E>
            boost::future<bool> contains(const E &element) {
                return proxy::IQueueImpl::contains(to_data(element));
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param elements the vector that elements will be drained to.
            * @return number of elements drained.
            */
            template<typename E>
            boost::future<size_t> drain_to(std::vector<E> &elements) {
                return proxy::IQueueImpl::drain_to_data().then(boost::launch::sync,
                                                               [&](boost::future<std::vector<serialization::pimpl::data>> f) {
                                                                   return drain_items(std::move(f), elements);
                                                               });
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param maxElements upper limit to be filled to vector.
            * @param elements vector that elements will be drained to.
            * @return number of elements drained.
            */
            template<typename E>
            boost::future<size_t> drain_to(std::vector<E> &elements, size_t max_elements) {
                return proxy::IQueueImpl::drain_to_data(max_elements).then(boost::launch::sync,
                                                                           [&](boost::future<std::vector<serialization::pimpl::data>> f) {
                                                                               return drain_items(std::move(f),
                                                                                                  elements);
                                                                           });
            }

            /**
            * Returns immediately without waiting.
            *
            * @return removes head of the queue and returns it to user . If not available returns empty constructed shared_ptr.
            */
            template<typename E>
            boost::future<boost::optional<E>> poll() {
                return poll<E>(std::chrono::milliseconds(0));
            }

            /**
            * Returns immediately without waiting.
            *
            * @return head of queue without removing it. If not available returns empty constructed shared_ptr.
            */
            template<typename E>
            boost::future<boost::optional<E>> peek() {
                return to_object<E>(proxy::IQueueImpl::peek_data());
            }

            /**
            *
            * @returns all elements as std::vector
            */
            template<typename E>
            boost::future<std::vector<E>> to_array() {
                return to_object_vector<E>(proxy::IQueueImpl::to_array_data());
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this queue contains all elements given in vector.
            */
            template<typename E>
            boost::future<bool> contains_all(const std::vector<E> &elements) {
                return proxy::IQueueImpl::contains_all_data(to_data_collection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to queue.
            */
            template<typename E>
            boost::future<bool> add_all(const std::vector<E> &elements) {
                return proxy::IQueueImpl::add_all_data(to_data_collection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            */
            template<typename E>
            boost::future<bool> remove_all(const std::vector<E> &elements) {
                return proxy::IQueueImpl::remove_all_data(to_data_collection(elements));
            }

            /**
            *
            * Removes the elements from this queue that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            */
            template<typename E>
            boost::future<bool> retain_all(const std::vector<E> &elements) {
                return proxy::IQueueImpl::retain_all_data(to_data_collection(elements));
            }

        private:
            iqueue(const std::string &instance_name, spi::ClientContext *context) : proxy::IQueueImpl(instance_name,
                                                                                                      context) {}

            template<typename E>
            size_t drain_items(boost::future<std::vector<serialization::pimpl::data>> f, std::vector<E> &elements) {
                auto datas = f.get();
                auto size = datas.size();
                elements.reserve(size);
                auto &ss = get_context().get_serialization_service();
                for (auto &data : datas) {
                    elements.push_back(ss.template to_object<E>(data).value());
                }
                return size;
            }
        };
    }
}

