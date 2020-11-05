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

#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/proxy/IQueueImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
    namespace client {
        /**
        * Concurrent, blocking, distributed, observable, client queue.
        *
        */
        class IQueue : public proxy::IQueueImpl {
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
            boost::future<boost::uuids::uuid> addItemListener(ItemListener &&listener, bool includeValue) {
                std::unique_ptr<impl::ItemEventHandler<protocol::codec::queue_addlistener_handler>> itemEventHandler(
                        new impl::ItemEventHandler<protocol::codec::queue_addlistener_handler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                std::move(listener),
                                includeValue));

                return proxy::IQueueImpl::addItemListener(std::move(itemEventHandler), includeValue);
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
                return proxy::IQueueImpl::put(toData(element));
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
                return proxy::IQueueImpl::offer(toData(element), timeout);
            }

            /**
            *
            * @return the head of the queue. If queue is empty waits for an item to be added.
            */
            template<typename E>
            boost::future<boost::optional<E>> take() {
                return poll<E>(std::chrono::milliseconds(-1));
            }

            /**
            *
            * @param timeout time to wait if item is not available.
            * @return the head of the queue. If queue is empty waits for specified time.
            */
            template<typename E>
            boost::future<boost::optional<E>> poll(std::chrono::milliseconds timeout) {
                return toObject<E>(proxy::IQueueImpl::pollData(timeout));
            }

            /**
            *
            * @param element to be removed.
            * @return true if element removed successfully.
            */
            template<typename E>
            boost::future<bool> remove(const E &element) {
                return proxy::IQueueImpl::remove(toData(element));
            }

            /**
            *
            * @param element to be checked.
            * @return true if queue contains the element.
            */
            template<typename E>
            boost::future<bool> contains(const E &element) {
                return proxy::IQueueImpl::contains(toData(element));
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param elements the vector that elements will be drained to.
            * @return number of elements drained.
            */
            template<typename E>
            boost::future<size_t> drainTo(std::vector<E> &elements) {
                return proxy::IQueueImpl::drainToData().then(boost::launch::deferred, [&](boost::future<std::vector<serialization::pimpl::Data>> f)  {
                    return drainItems(std::move(f), elements);
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
            boost::future<size_t> drainTo(std::vector<E> &elements, size_t maxElements) {
                return proxy::IQueueImpl::drainToData(maxElements).then(boost::launch::deferred, [&](boost::future<std::vector<serialization::pimpl::Data>> f)  {
                    return drainItems(std::move(f), elements);
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
                return toObject<E>(proxy::IQueueImpl::peekData());
            }

            /**
            *
            * @returns all elements as std::vector
            */
            template<typename E>
            boost::future<std::vector<E>> toArray() {
                return toObjectVector<E>(proxy::IQueueImpl::toArrayData());
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this queue contains all elements given in vector.
            */
            template<typename E>
            boost::future<bool> containsAll(const std::vector<E> &elements) {
                return proxy::IQueueImpl::containsAllData(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to queue.
            */
            template<typename E>
            boost::future<bool> addAll(const std::vector<E> &elements) {
                return proxy::IQueueImpl::addAllData(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            */
            template<typename E>
            boost::future<bool> removeAll(const std::vector<E> &elements) {
                return proxy::IQueueImpl::removeAllData(toDataCollection(elements));
            }

            /**
            *
            * Removes the elements from this queue that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            */
            template<typename E>
            boost::future<bool> retainAll(const std::vector<E> &elements) {
                return proxy::IQueueImpl::retainAllData(toDataCollection(elements));
            }

        private:
            IQueue(const std::string &instanceName, spi::ClientContext *context) : proxy::IQueueImpl(instanceName,
                                                                                                     context) {}

            template<typename E>
            size_t drainItems(boost::future<std::vector<serialization::pimpl::Data>> f, std::vector<E> &elements) {
                auto datas = f.get();
                auto size = datas.size();
                elements.reserve(size);
                auto &ss = getContext().getSerializationService();
                for (auto &data : datas) {
                    elements.push_back(ss.template toObject<E>(data).value());
                }
                return size;
            }
        };
    }
}

