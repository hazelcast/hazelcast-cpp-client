#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/pimpl/IQueueImpl.h"
#include <stdexcept>

namespace hazelcast {
    namespace client {

        /**
        * Concurrent, blocking, distributed, observable, client queue.
        *
        * @tparam E item type
        */
        template<typename E>
        class HAZELCAST_API IQueue : public DistributedObject {
            friend class HazelcastClient;

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
                spi::ClusterService& cs = getContext().getClusterService();
                serialization::pimpl::SerializationService& ss = getContext().getSerializationService();
                impl::ItemEventHandler<E> *itemEventHandler = new impl::ItemEventHandler<E>(getName(), cs, ss, listener, includeValue);
                return impl->addItemListener(itemEventHandler, includeValue);
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
                return impl->removeItemListener(registrationId);
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
                return impl->offer(toData(element), timeoutInMillis);
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
                return toObject<E>(impl->poll(timeoutInMillis));
            }

            /**
            *
            * @return remaining capacity
            */
            int remainingCapacity() {
                return impl->remainingCapacity();
            }

            /**
            *
            * @param element to be removed.
            * @return true if element removed successfully.
            */
            bool remove(const E& element) {
                return impl->remove(toData(element));
            }

            /**
            *
            * @param element to be checked.
            * @return true if queue contains the element.
            */
            bool contains(const E& element) {
                return impl->contains(toData(element));
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param elements the vector that elements will be drained to.
            * @return number of elements drained.
            */
            int drainTo(std::vector<E>& elements) {
                return drainTo(elements, -1);
            }

            /**
            * Note that elements will be pushed_back to vector.
            *
            * @param maxElements upper limit to be filled to vector.
            * @param elements vector that elements will be drained to.
            * @return number of elements drained.
            */
            int drainTo(std::vector<E>& elements, int maxElements) {
                std::vector<serialization::pimpl::Data> coll = impl->drainTo(maxElements);
                for (std::vector<serialization::pimpl::Data>::const_iterator it = coll.begin(); it != coll.end(); ++it) {
                    boost::shared_ptr<E> e = getContext().getSerializationService().template toObject<E>(*it);
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
                return toObject<E>(impl->peek());
            }

            /**
            *
            * @return size of this distributed queue
            */
            int size() {
                return impl->size();
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
                return getObjectList(impl->toArray());
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this queue contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> list = getDataList(elements);
                return impl->containsAll(list);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to queue.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = getDataList(elements);
                return impl->addAll(dataList);
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = getDataList(elements);
                return impl->removeAll(dataList);
            }

            /**
            *
            * Removes the elements from this queue that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataList = getDataList(elements);
                return impl->retainAll(dataList);
            }

            /**
            * Removes all elements from queue.
            */
            void clear() {
                impl->clear();
            }

            /**
            * Destructor
            */
            ~IQueue() {
                delete impl;
            }

        private:

            pimpl::IQueueImpl *impl;
            int partitionId;

            IQueue(const std::string& instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:queueService", instanceName, context)
            , impl(new pimpl::IQueueImpl(instanceName, context)) {
                {
                    serialization::pimpl::Data data = toData<std::string>(getName());
                    partitionId = getPartitionId(data);
                }
            }

            template<typename T>
            serialization::pimpl::Data toData(const T& object) {
                return getContext().getSerializationService().template toData<T>(&object);
            }

            template<typename T>
            boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                return getContext().getSerializationService().template toObject<T>(data);
            }


            std::vector<serialization::pimpl::Data> getDataList(const std::vector<E>& objects) {
                int size = objects.size();
                std::vector<serialization::pimpl::Data> dataList(size);
                for (int i = 0; i < size; i++) {
                    dataList[i] = toData(objects[i]);
                }
                return dataList;
            }

            std::vector<E> getObjectList(const std::vector<serialization::pimpl::Data>& dataList) {
                size_t size = dataList.size();
                std::vector<E> objects(size);
                for (size_t i = 0; i < size; i++) {
                    boost::shared_ptr<E> object = getContext().getSerializationService().template toObject<E>(dataList[i]);
                    objects[i] = *object;
                }
                return objects;
            }

            void onDestroy() {
            }
        };
    }
}

#endif /* HAZELCAST_IQUEUE */

