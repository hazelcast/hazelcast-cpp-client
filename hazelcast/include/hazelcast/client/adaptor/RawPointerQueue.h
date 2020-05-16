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
#include <memory>

#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/impl/DataArrayImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * Concurrent, blocking, distributed, observable, client queue.
            *
            * @tparam T item type
            */
            template<typename T>
            class RawPointerQueue {
            public:
                RawPointerQueue(IQueue<T> &q) : queue(q), serializationService(q.getContext().getSerializationService()) {
                }

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
                std::string addItemListener(ItemListener<T> &listener, bool includeValue) {
                    return queue.addItemListener(listener, includeValue);
                }

                /**
                * Removes the specified item listener.
                * Returns silently if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeItemListener(const std::string &registrationId) {
                    return queue.removeItemListener(registrationId);
                }

                /**
                * Inserts the specified element into this queue.
                *
                * @param element to add
                * @return <tt>true</tt> if the element was added to this queue, else
                *         <tt>false</tt>
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool offer(const T &element) {
                    return queue.offer(element, 0);
                }

                /**
                * Puts the element into queue.
                * If queue is  full waits for space to became available.
                */
                void put(const T &e) {
                    queue.offer(e, -1);
                }

                /**
                * Inserts the specified element into this queue.
                * If queue is  full waits for space to became available for specified time.
                *
                * @param element to add
                * @param timeoutInMillis how long to wait before giving up, in units of
                * @return <tt>true</tt> if successful, or <tt>false</tt> if
                *         the specified waiting time elapses before space is available
                */
                bool offer(const T &element, long timeoutInMillis) {
                    return queue.offer(element, timeoutInMillis);
                }

                /**
                *
                * @return the head of the queue. If queue is empty waits for an item to be added.
                */
                std::unique_ptr<T> take() {
                    return serializationService.toObject<T>(queue.pollData(-1).get());
                }

                /**
                *
                * @param timeoutInMillis time to wait if item is not available.
                * @return the head of the queue. If queue is empty waits for specified time.
                */
                std::unique_ptr<T> poll(long timeoutInMillis) {
                    return serializationService.toObject<T>(queue.pollData(timeoutInMillis).get());
                }

                /**
                *
                * @return remaining capacity
                */
                int remainingCapacity() {
                    return queue.remainingCapacity();
                }

                /**
                *
                * @param element to be removed.
                * @return true if element removed successfully.
                */
                bool remove(const T &element) {
                    return queue.remove(element);
                }

                /**
                *
                * @param element to be checked.
                * @return true if queue contains the element.
                */
                bool contains(const T &element) {
                    return queue.contains(element);
                }

                /**
                *
                * @return The elements in the queue.
                */
                std::unique_ptr<DataArray<T> > drainTo() {
                    return std::unique_ptr<DataArray<T> >(new hazelcast::client::impl::DataArrayImpl<T>(queue.drainToData(-1), serializationService));
                }

                /**
                *
                * @param maxElements upper limit to be filled.
                * @return The elements in the queue.
                */
                std::unique_ptr<DataArray<T> > drainTo(size_t maxElements) {
                    return std::unique_ptr<DataArray<T> >(new hazelcast::client::impl::DataArrayImpl<T>(queue.drainToData(maxElements), serializationService));
                }

                /**
                * Returns immediately without waiting.
                *
                * @return removes head of the queue and returns it to user . If not available returns empty constructed shared_ptr.
                */
                std::unique_ptr<T> poll() {
                    return serializationService.toObject<T>(queue.pollData(0).get());
                }

                /**
                * Returns immediately without waiting.
                *
                * @return head of queue without removing it. If not available returns empty constructed shared_ptr.
                */
                std::unique_ptr<T> peek() {
                    return serializationService.toObject<T>(queue.peekData().get());
                }

                /**
                *
                * @return size of this distributed queue
                */
                int size() {
                    return queue.size();
                }

                /**
                *
                * @return true if queue is empty
                */
                bool isEmpty() {
                    return queue.isEmpty();
                }

                /**
                *
                * @returns all elements
                */
                std::unique_ptr<DataArray<T> > toArray() {
                    return std::unique_ptr<DataArray<T> >(new hazelcast::client::impl::DataArrayImpl<T>(queue.toArrayData(), serializationService));
                }

                /**
                *
                * @param elements The items to be searched for in the queue.
                * @return true if this queue contains all item given in elements.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool containsAll(const std::vector<T> &elements) {
                    return queue.containsAll(elements);
                }

                /**
                *
                * @param elements he items to be inserted into the queue.
                * @return true if all elements given in vector can be added to queue.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool addAll(const std::vector<T> &elements) {
                    return queue.addAll(elements);
                }

                /**
                *
                * @param elements The items to be removed from the queue
                * @return true if all elements are removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool removeAll(const std::vector<T> &elements) {
                    return queue.removeAll(elements);
                }

                /**
                *
                * Removes the elements from this queue that are not available in given "elements" vector
                * @param elements The items to be retained in the queue
                * @return true if operation is successful.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool retainAll(const std::vector<T> &elements) {
                    return queue.retainAll(elements);
                }

                /**
                * Removes all elements from queue.
                */
                void clear() {
                    queue.clear();
                }

            private:
                IQueue<T> &queue;
                serialization::pimpl::SerializationService &serializationService;
            };
        }

    }
}


