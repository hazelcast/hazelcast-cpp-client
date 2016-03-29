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
#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include "hazelcast/client/proxy/IQueueImpl.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include <stdexcept>
#include "hazelcast/client/protocol/codec/QueueAddListenerCodec.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template <typename E>
            class RawPointerQueue;
        }

        /**
        * Concurrent, blocking, distributed, observable, client queue.
        *
        * @tparam E item type
        */
        template<typename E>
        class IQueue : public proxy::IQueueImpl {
            friend class HazelcastClient;
            friend class adaptor::RawPointerQueue<E>;

        public:
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
            std::string addItemListener(ItemListener<E>& listener, bool includeValue) {
                spi::ClusterService& cs = context->getClusterService();
                serialization::pimpl::SerializationService& ss = context->getSerializationService();
                impl::ItemEventHandler<E, protocol::codec::QueueAddListenerCodec::AbstractEventHandler> *itemEventHandler =
                        new impl::ItemEventHandler<E, protocol::codec::QueueAddListenerCodec::AbstractEventHandler>(getName(), cs, ss, listener, includeValue);
                return proxy::IQueueImpl::addItemListener(itemEventHandler, includeValue);
            }

            /**
            * Removes the specified item listener.
            * Returns silently if the specified listener is not added before.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeItemListener(const std::string& registrationId) {
                return proxy::IQueueImpl::removeItemListener(registrationId);
            }

            /**
            * Inserts the specified element into this queue.
            *
            * @param element to add
            * @return <tt>true</tt> if the element was added to this queue, else
            *         <tt>false</tt>
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool offer(const E& element) {
                return offer(element, 0);
            }

            /**
            * Puts the element into queue.
            * If queue is  full waits for space to became available.
            */
            void put(const E& e) {
                offer(e, -1);
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
            bool offer(const E& element, long timeoutInMillis) {
                return proxy::IQueueImpl::offer(toData(element), timeoutInMillis);
            }

            /**
            *
            * @return the head of the queue. If queue is empty waits for an item to be added.
            */
            boost::shared_ptr<E> take() {
                return poll(-1);
            }

            /**
            *
            * @param timeoutInMillis time to wait if item is not available.
            * @return the head of the queue. If queue is empty waits for specified time.
            */
            boost::shared_ptr<E> poll(long timeoutInMillis) {
                return boost::shared_ptr<E>(boost::shared_ptr<E>(toObject<E>(
                        proxy::IQueueImpl::pollData(timeoutInMillis))));
            }

            /**
            *
            * @return remaining capacity
            */
            int remainingCapacity() {
                return proxy::IQueueImpl::remainingCapacity();
            }

            /**
            *
            * @param element to be removed.
            * @return true if element removed successfully.
            */
            bool remove(const E& element) {
                return proxy::IQueueImpl::remove(toData(element));
            }

            /**
            *
            * @param element to be checked.
            * @return true if queue contains the element.
            */
            bool contains(const E& element) {
                return proxy::IQueueImpl::contains(toData(element));
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param elements the vector that elements will be drained to.
            * @return number of elements drained.
            */
            size_t drainTo(std::vector<E>& elements) {
                return drainTo(elements, -1);
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param maxElements upper limit to be filled to vector.
            * @param elements vector that elements will be drained to.
            * @return number of elements drained.
            */
            size_t drainTo(std::vector<E>& elements, size_t maxElements) {
                std::vector<serialization::pimpl::Data> coll = proxy::IQueueImpl::drainToData(maxElements);
                for (std::vector<serialization::pimpl::Data>::const_iterator it = coll.begin(); it != coll.end(); ++it) {
                    std::auto_ptr<E> e = context->getSerializationService().template toObject<E>(*it);
                    elements.push_back(*e);
                }
                return coll.size();
            }

            /**
            * Returns immediately without waiting.
            *
            * @return removes head of the queue and returns it to user . If not available returns empty constructed shared_ptr.
            */
            boost::shared_ptr<E> poll() {
                return poll(0);
            }

            /**
            * Returns immediately without waiting.
            *
            * @return head of queue without removing it. If not available returns empty constructed shared_ptr.
            */
            boost::shared_ptr<E> peek() {
                return boost::shared_ptr<E>(toObject<E>(proxy::IQueueImpl::peekData()));
            }

            /**
            *
            * @return size of this distributed queue
            */
            int size() {
                return proxy::IQueueImpl::size();
            }

            /**
            *
            * @return true if queue is empty
            */
            bool isEmpty() {
                return size() == 0;
            }

            /**
            *
            * @returns all elements as std::vector
            */
            std::vector<E> toArray() {
                return toObjectCollection<E>(proxy::IQueueImpl::toArrayData());
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this queue contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> list = toDataCollection(elements);
                return proxy::IQueueImpl::containsAll(list);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to queue.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = toDataCollection(elements);
                return proxy::IQueueImpl::addAll(dataList);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = toDataCollection(elements);
                return proxy::IQueueImpl::removeAll(dataList);
            }

            /**
            *
            * Removes the elements from this queue that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = toDataCollection(elements);
                return proxy::IQueueImpl::retainAll(dataList);
            }

            /**
            * Removes all elements from queue.
            */
            void clear() {
                proxy::IQueueImpl::clear();
            }
        private:
            IQueue(const std::string& instanceName, spi::ClientContext *context)
            : proxy::IQueueImpl(instanceName, context) {
            }
        };
    }
}

#endif /* HAZELCAST_IQUEUE */

