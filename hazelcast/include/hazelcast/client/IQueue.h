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
#include "hazelcast/client/exception/NoSuchElementException.h"
#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/DistributedObject.h"
#include <stdexcept>

namespace hazelcast {
    namespace client {


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

            bool add(const E &e) {
                if (offer(e)) {
                    return true;
                }
                throw exception::IllegalStateException("bool IQueue::dd(const E& e)", "Queue is full!");
            };

            /**
             * Inserts the specified element into this queue if it is possible to do
             * so immediately without violating capacity restrictions, returning
             * <tt>true</tt> upon success and <tt>false</tt> if no space is currently
             * available.
             *
             * @param e the element to add
             * @return <tt>true</tt> if the element was added to this queue, else
             *         <tt>false</tt>
             */
            bool offer(const E &e) {
                try {
                    return offer(e, 0);
                } catch (exception::InterruptedException &ex) {
                    return false;
                }
            };

            void put(const E &e) {
                offer(e, -1);
            };

            /**
            * Inserts the specified element into this queue, waiting up to the
            * specified wait time if necessary for space to become available.
            *
            * @param e the element to add
            * @param timeout how long to wait before giving up, in units of
            *        
            * @param unit a <tt>TimeUnit</tt> determining how to interpret the
            *        <tt>timeout</tt> parameter
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

            boost::shared_ptr<E> take() {
                return poll(-1);
            };

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

            int remainingCapacity() {
                queue::RemainingCapacityRequest *request = new queue::RemainingCapacityRequest(getName());
                boost::shared_ptr<int> cap = invoke<int>(request, partitionId);
                return *cap;
            };

            bool remove(const E &o) {
                serialization::Data data = toData(o);
                queue::RemoveRequest *request = new queue::RemoveRequest(getName(), data);
                bool result = *(invoke<bool>(request, partitionId));
                return result;
            };

            bool contains(const E &o) {
                std::vector<serialization::Data> list(1);
                list[0] = toData(o);
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), list);
                return *(invoke<bool>(request, partitionId));
            };

            int drainTo(std::vector<E> &objects) {
                return drainTo(objects, -1);
            };

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

            boost::shared_ptr<E> remove() {
                boost::shared_ptr<E> res = poll();
                if (res == NULL) {
                    throw exception::NoSuchElementException("E IQueue::remove()", "Queue is empty!");
                }
                return res;
            };

            boost::shared_ptr<E> poll() {
                try {
                    return poll(0);
                } catch (exception::InterruptedException &) {
                    return boost::shared_ptr<E>();
                }
            };

            boost::shared_ptr<E> element() {
                boost::shared_ptr<E> res = peek();
                if (res == NULL) {
                    throw exception::NoSuchElementException("E IQueue::element()", "Queue is empty!");
                }
                return res;
            };

            boost::shared_ptr<E> peek() {
                queue::PeekRequest *request = new queue::PeekRequest(getName());
                return invoke<E>(request, partitionId);
            };

            int size() {
                queue::SizeRequest *request = new queue::SizeRequest(getName());
                boost::shared_ptr<int> size = invoke<int>(request, partitionId);
                return *size;
            }

            bool isEmpty() {
                return size() == 0;
            };

            std::vector<E> toArray() {
                queue::IteratorRequest *request = new queue::IteratorRequest(getName());
                boost::shared_ptr<impl::PortableCollection> result = invoke<impl::PortableCollection>(request, partitionId);
                std::vector<serialization::Data> const &coll = result->getCollection();
                return getObjectList(coll);
            };

            bool containsAll(const std::vector<E> &c) {
                std::vector<serialization::Data> list = getDataList(c);
                queue::ContainsRequest *request = new queue::ContainsRequest(getName(), list);
                boost::shared_ptr<bool> contains = invoke<bool>(request, partitionId);
                return *contains;
            }

            bool addAll(const std::vector<E> &c) {
                std::vector<serialization::Data> dataList = getDataList(c);
                queue::AddAllRequest *request = new queue::AddAllRequest(getName(), dataList);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

            bool removeAll(const std::vector<E> &c) {
                std::vector<serialization::Data> dataList = getDataList(c);
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), dataList, false);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

            bool retainAll(const std::vector<E> &c) {
                std::vector<serialization::Data> dataList = getDataList(c);
                queue::CompareAndRemoveRequest *request = new queue::CompareAndRemoveRequest(getName(), dataList, true);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            }

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