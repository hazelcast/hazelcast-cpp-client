#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include "hazelcast/client/queue/OfferRequest.h"
#include "hazelcast/client/queue/PollRequest.h"
#include "hazelcast/client/queue/RemainingCapacityRequest.h"
#include "hazelcast/client/queue/RemoveRequest.h"
#include "hazelcast/client/queue/ContainsRequest.h"
#include "hazelcast/client/queue/DrainRequest.h"
#include "hazelcast/client/queue/PeekRequest.h"
#include "hazelcast/client/queue/SizeRequest.h"
#include "hazelcast/client/queue/CompareAndRemoveRequest.h"
#include "hazelcast/client/queue/AddAllRequest.h"
#include "hazelcast/client/queue/ClearRequest.h"
#include "hazelcast/client/queue/IteratorRequest.h"
#include "hazelcast/client/queue/AddListenerRequest.h"
#include "hazelcast/client/queue/RemoveListenerRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/ItemEvent.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/exception/InterruptedException.h"
#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/DistributedObject.h"
#include <stdexcept>

namespace hazelcast {
    namespace client {

        /**
         * Concurrent, blocking, distributed, observable, client queue.
         *
         * @param <E> item type
         */
        template<typename E>
        class HAZELCAST_API IQueue : public DistributedObject {
            friend class HazelcastClient;

        public:
            /**
             * Adds an item listener for this collection. Listener will get notified
             * for all collection add/remove events.
             *
             * @param listener     item listener
             * @param includeValue <tt>true</tt> updated item should be passed
             *                     to the item listener, <tt>false</tt> otherwise.
             * @return returns registration id.
             */
            template < typename L>
            std::string addItemListener(L &listener, bool includeValue) {
                queue::AddListenerRequest *request = new queue::AddListenerRequest(getName(), includeValue);
                spi::ClusterService &cs = getContext().getClusterService();
                serialization::SerializationService &ss = getContext().getSerializationService();
                impl::ItemEventHandler<E, L> *entryEventHandler = new impl::ItemEventHandler<E, L>(getName(), cs, ss, listener, includeValue);
                return listen(request, entryEventHandler);
            };

            /**
             * Removes the specified item listener.
             * Returns silently if the specified listener is not added before.
             *
             * @param registrationId Id of listener registration.
             *
             * @return true if registration is removed, false otherwise
             */
            bool removeItemListener(const std::string &registrationId) {
                queue::RemoveListenerRequest *request = new queue::RemoveListenerRequest(getName(), registrationId);
                return stopListening(request, registrationId);
            };

            /**
             *
             * @param element E
             * @return true if element is added successfully.
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             * @throws IllegalStateException if queue is full.
             */
            bool add(const E &element) {
                if (offer(element)) {
                    return true;
                }
                throw exception::IllegalStateException("bool IQueue::add(const E& e)", "Queue is full!");
            };

            /**
             * Inserts the specified element into this queue.
             *
             * @param element to add
             * @return <tt>true</tt> if the element was added to this queue, else
             *         <tt>false</tt>
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool offer(const E &e) {
                try {
                    return offer(e, 0);
                } catch (exception::InterruptedException &ex) {
                    return false;
                }
            };

            /**
             * Puts the element into queue.
             * If queue is  full waits for space to became available.
             */
            void put(const E &e) {
                offer(e, -1);
            };

            /**
             * Inserts the specified element into this queue.
             * If queue is  full waits for space to became available for specified time.
             *
             * @param e the element to add
             * @param timeout how long to wait before giving up, in units of
             * @return <tt>true</tt> if successful, or <tt>false</tt> if
             *         the specified waiting time elapses before space is available
             * @throws InterruptedException if interrupted while waiting
             */
            bool offer(const E &e, long timeoutInMillis) {
                serialization::Data data = toData(e);
                queue::OfferRequest *request = new queue::OfferRequest(getName(), data, timeoutInMillis);
                bool result;
                try {
                    result = *(invoke<bool>(request, partitionId));
                } catch(exception::ServerException &e) {
                    throw exception::InterruptedException("IQueue::offer", "timeout");
                }
                return result;
            };

            /**
             *
             * @return the head of the queue. If queue is empty waits for an item to be added.
             */
            boost::shared_ptr<E> take() {
                return poll(-1);
            };

            /**
             *
             * @param long timeoutInMillis time to wait if item is not available.
             * @return the head of the queue. If queue is empty waits for specified time.
             */
            boost::shared_ptr<E> poll(long timeoutInMillis) {
                queue::PollRequest *request = new queue::PollRequest(getName(), timeoutInMillis);
                boost::shared_ptr<E> result;
                try {
                    result = invoke<E>(request, partitionId);
                } catch(exception::ServerException &) {
                    throw exception::InterruptedException("IQueue::poll", "timeout");
                }
                return result;
            };

            /**
             *
             * @return remaining capacity
             */
            int remainingCapacity() {
                queue::RemainingCapacityRequest *request = new queue::RemainingCapacityRequest(getName());
                boost::shared_ptr<int> cap = invoke<int>(request, partitionId);
                return *cap;
            };

            /**
             *
             * @param element to be removed.
             * @return true if element removed successfully.
             */
            bool remove(const E &element) {
                serialization::Data data = toData(element);
                queue::RemoveRequest *request = new queue::RemoveRequest(getName(), data);
                bool result = *(invoke<bool>(request, partitionId));
                return result;
            };

            /**
             *
             * @param element to be checked.
             * @return true if queue contains the element.
             */
            bool contains(const E &element) {
                std::vector<serialization::Data> list(1);
                list[0] = toData(element);
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), list);
                return *(invoke<bool>(request, partitionId));
            };

            /**
             * Note that elements will be pushed_back to vector.
             *
             * @param vector that elements will be drained to.
             * @return number of elements drained.
             */
            int drainTo(std::vector<E> &elements) {
                return drainTo(elements, -1);
            };

            /**
             * Note that elements will be pushed_back to vector.
             *
             * @param maxElements upper limit to be filled to vector.
             * @param vector that elements will be drained to.
             * @return number of elements drained.
             */
            int drainTo(std::vector<E> &c, int maxElements) {
                queue::DrainRequest *request = new queue::DrainRequest(getName(), maxElements);
                boost::shared_ptr<impl::PortableCollection> result = invoke<impl::PortableCollection>(request, partitionId);
                const std::vector<serialization::Data> &coll = result->getCollection();
                for (std::vector<serialization::Data>::const_iterator it = coll.begin(); it != coll.end(); ++it) {
                    boost::shared_ptr<E> e = getContext().getSerializationService().template toObject<E>(*it);
                    c.push_back(*e);
                }
                return coll.size();
            };

            /**
             * Returns immediately without waiting.
             *
             * @return removes head of the queue and returns it to user . If not available returns empty constructed shared_ptr.
             */
            boost::shared_ptr<E> poll() {
                try {
                    return poll(0);
                } catch (exception::InterruptedException &) {
                    return boost::shared_ptr<E>();
                }
            };

            /**
             * Returns immediately without waiting.
             *
             * @return head of queue without removing it. If not available returns empty constructed shared_ptr.
             */
            boost::shared_ptr<E> peek() {
                queue::PeekRequest *request = new queue::PeekRequest(getName());
                return invoke<E>(request, partitionId);
            };

            /**
             *
             * @return size of this distributed queue
             */
            int size() {
                queue::SizeRequest *request = new queue::SizeRequest(getName());
                boost::shared_ptr<int> size = invoke<int>(request, partitionId);
                return *size;
            }

            /**
             *
             * @return true if queue is empty
             */
            bool isEmpty() {
                return size() == 0;
            };

            /**
             *
             * @returns all elements as std::vector
             */
            std::vector<E> toArray() {
                queue::IteratorRequest *request = new queue::IteratorRequest(getName());
                boost::shared_ptr<impl::PortableCollection> result = invoke<impl::PortableCollection>(request, partitionId);
                std::vector<serialization::Data> const &coll = result->getCollection();
                return getObjectList(coll);
            };

            /**
             *
             * @param elements std::vector<E>
             * @return true if this queue contains all elements given in vector.
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool containsAll(const std::vector<E> &elements) {
                std::vector<serialization::Data> list = getDataList(elements);
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), list);
                boost::shared_ptr<bool> contains = invoke<bool>(request, partitionId);
                return *contains;
            }

            /**
             *
             * @param elements std::vector<E>
             * @return true if all elements given in vector can be added to queue.
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool addAll(const std::vector<E> &elements) {
                std::vector<serialization::Data> dataList = getDataList(elements);
                queue::AddAllRequest *request = new queue::AddAllRequest(getName(), dataList);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

            /**
             *
             * @param elements std::vector<E>
             * @return true if all elements are removed successfully.
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool removeAll(const std::vector<E> &c) {
                std::vector<serialization::Data> dataList = getDataList(c);
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), dataList, false);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

            /**
             *
             * Removes the elements from this queue that are not available in given "elements" vector
             * @param elements std::vector<E>
             * @return true if operation is successful.
             * @throws ClassCastException if the type of the specified element is incompatible with the server side.
             */
            bool retainAll(const std::vector<E> &c) {
                std::vector<serialization::Data> dataList = getDataList(c);
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), dataList, true);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

            /**
             * Removes all elements from queue.
             */
            void clear() {
                queue::ClearRequest *request = new queue::ClearRequest(getName());
                invoke<bool>(request, partitionId);
            };

        private:

            int partitionId;

            IQueue(const std::string &instanceName, spi::ClientContext *context)
            :DistributedObject("hz:impl:queueService", instanceName, context) {
                serialization::Data data = toData<std::string>(getName());
                partitionId = getPartitionId(data);
            };

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };


            std::vector<serialization::Data> getDataList(const std::vector<E> &objects) {
                std::vector<serialization::Data> dataList(objects.size());
                for (int i = 0; i < objects.size(); i++) {
                    dataList[i] = toData(objects[i]);
                }
                return dataList;
            };

            std::vector<E> getObjectList(const std::vector<serialization::Data> &dataList) {
                std::vector<E> objects(dataList.size());
                for (int i = 0; i < dataList.size(); i++) {
                    boost::shared_ptr<E> object = getContext().getSerializationService(). template toObject<E>(dataList[i]);
                    objects[i] = *object;
                }
                return objects;
            };

            void onDestroy() {
            };


        };
    }
}

#endif /* HAZELCAST_IQUEUE */