#ifndef HAZELCAST_IQUEUE
#define HAZELCAST_IQUEUE

#include "queue/OfferRequest.h"
#include "queue/PollRequest.h"
#include "queue/RemainingCapacityRequest.h"
#include "queue/RemoveRequest.h"
#include "queue/ContainsRequest.h"
#include "queue/DrainRequest.h"
#include "queue/PeekRequest.h"
#include "queue/SizeRequest.h"
#include "queue/CompareAndRemoveRequest.h"
#include "queue/AddAllRequest.h"
#include "queue/ClearRequest.h"
#include "queue/DestroyRequest.h"
#include "queue/ItearatorRequest.h"
#include "queue/AddListenerRequest.h"
#include "impl/PortableCollection.h"
#include "impl/ItemEventHandler.h"
#include "ItemEvent.h"
#include "IllegalStateException.h"
#include "InterruptedException.h"
#include "NoSuchElementException.h"
#include "hazelcast/client/spi/DistributedObjectListenerService.h"
#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename E>
        class IQueue {
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
            long addItemListener(L& listener, bool includeValue) {
                queue::AddListenerRequest request(instanceName, includeValue);
                impl::ItemEvent<E> entryEventHandler(instanceName, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<queue::AddListenerRequest, impl::ItemEventHandler<E, L>, impl::PortableItemEvent >(instanceName, request, entryEventHandler);
            };

            /**
            * Removes the specified item listener.
            * Returns silently if the specified listener is not added before.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeItemListener(long registrationId) {
                return context->getServerListenerService().stopListening(instanceName, registrationId);
            };

            bool add(const E& e) {
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
            bool offer(const E& e) {
                try {
                    return offer(e, 0);
                } catch (exception::InterruptedException& ex) {
                    return false;
                }
            };

            void put(const E& e) {
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
            bool offer(const E& e, long timeoutInMillis) {
                serialization::Data data = toData(e);
                queue::OfferRequest request(instanceName, timeoutInMillis, data);
                bool result;
                try {
                    result = invoke<bool>(request);
                } catch(exception::ServerException& e){
                    throw exception::InterruptedException("IQueue::offer", "timeout");
                }
                return result;
            };

            E take() {
                return poll(-1);
            };

            E poll(long timeoutInMillis) {
                queue::PollRequest request(instanceName, timeoutInMillis);
                E result;
                try {
                    result = invoke<E>(request);
                } catch(exception::ServerException& e){
                    throw exception::InterruptedException("IQueue::poll", "timeout");
                }
                return result;
            };

            int remainingCapacity() {
                queue::RemainingCapacityRequest request(instanceName);
                return invoke<int>(request);
            };

            bool remove(const E& o) {
                serialization::Data data = toData(o);
                queue::RemoveRequest request(instanceName, data);
                bool result = invoke(request);
                return result;
            };

            bool contains(const E& o) {
                std::vector<serialization::Data> list(1);
                list[0] = toData(o);
                queue::ContainsRequest request(instanceName, list);
                return invoke<bool>(request);
            };

            int drainTo(const std::vector<E>& objects) {
                return drainTo(objects, -1);
            };

            int drainTo(const std::vector<E>& c, int maxElements) {
                queue::DrainRequest request(instanceName, maxElements);
                impl::PortableCollection result = invoke<impl::PortableCollection>(request);
                const std::vector<serialization::Data>& coll = result.getCollection();
                for (std::vector<serialization::Data>::const_iterator it = coll.begin(); it != coll.end(); ++it) {
                    E e = context->getSerializationService().template toObject<E>(*it);
                    c.add(e);
                }
                return coll.size();
            };

            E remove() {
                E res = poll();
                if (res == E()) {
                    throw exception::NoSuchElementException("E IQueue::remove()", "Queue is empty!");
                }
                return res;
            };

            E poll() {
                try {
                    return poll(0);
                } catch (exception::InterruptedException& e) {
                    return E();
                }
            };

            E element() {
                E res = peek();
                if (res == E()) {
                    throw exception::NoSuchElementException("E IQueue::element()", "Queue is empty!");
                }
                return res;
            };

            E peek() {
                queue::PeekRequest request(instanceName);
                return invoke<E>(request);
            };

            int size() {
                queue::SizeRequest request(instanceName);
                return invoke<int>(request);;
            }

            bool isEmpty() {
                return size() == 0;
            };

            std::vector<E> toArray() {
                queue::IteratorRequest request(instanceName);
                impl::PortableCollection result = invoke<impl::PortableCollection>(request);
                const vector<serialization::Data> const & coll = result.getCollection();
                return getObjectList(coll);
            };

            bool containsAll(const std::vector<E>& c) {
                std::vector<serialization::Data> list = getDataList(c);
                queue::ContainsRequest request(instanceName, list);
                return invoke<bool>(request);
            }

            bool addAll(const std::vector<E>& c) {
                vector<serialization::Data> dataList = getDataList(c);
                queue::AddAllRequest request(instanceName, dataList);
                return invoke<bool>(request);
            }

            bool removeAll(const std::vector<E>&c) {
                queue::CompareAndRemoveRequest request(instanceName, getDataList(c), false);
                return invoke<bool>(request);
            }

            bool retainAll(const std::vector<E>&c) {
                queue::CompareAndRemoveRequest request(instanceName, getDataList(c), true);
                return invoke<bool>(request);
            }


            void clear() {
                queue::ClearRequest request(instanceName);
                invoke<bool>(request);
            };

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            void destroy(){
                queue::DestroyRequest request(instanceName);
                invoke<bool>(request);
                context->getDistributedObjectListenerService().removeDistributedObject(instanceName);
            };

            std::string getName() {
                return instanceName;
            };


        private:
            std::string instanceName;
            serialization::Data key;
            spi::ClientContext *context;

            IQueue() {

            };

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                this->context = clientContext;
                this->instanceName = instanceName;
                key = context->getSerializationService().toData<std::string>(&instanceName);
            };

            template<typename T>
            serialization::Data toData(const T& object) {
                return context->getSerializationService().toData<T>(&object);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context->getInvocationService().template invokeOnKeyOwner<Response>(request, key);
            };

            std::vector<serialization::Data> getDataList(const std::vector<E>& objects) {
                std::vector<serialization::Data> dataList(objects.size());
                for (int i = 0; i < objects.size(); i++) {
                    dataList[i] = toData(objects[i]);
                }
                return dataList;
            };

            std::vector<E> getObjectList(const std::vector<serialization::Data>& dataList) {
                std::vector<E> objects(dataList.size());
                for (int i = 0; i < dataList.size(); i++) {
                    objects[i] = context->getSerializationService(). template toObject<E>(dataList[i]);
                }
                return objects;
            };

        };
    }
}

#endif /* HAZELCAST_IQUEUE */