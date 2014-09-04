#ifndef HAZELCAST_ILIST
#define HAZELCAST_ILIST

#include "hazelcast/client/collection/CollectionAddListenerRequest.h"
#include "hazelcast/client/collection/CollectionRemoveListenerRequest.h"
#include "hazelcast/client/collection/ListAddRequest.h"
#include "hazelcast/client/collection/ListRemoveRequest.h"
#include "hazelcast/client/collection/ListAddAllRequest.h"
#include "hazelcast/client/collection/ListGetRequest.h"
#include "hazelcast/client/collection/ListSetRequest.h"
#include "hazelcast/client/collection/ListIndexOfRequest.h"
#include "hazelcast/client/collection/ListSubRequest.h"
#include "hazelcast/client/collection/CollectionSizeRequest.h"
#include "hazelcast/client/collection/CollectionContainsRequest.h"
#include "hazelcast/client/collection/CollectionAddAllRequest.h"
#include "hazelcast/client/collection/CollectionGetAllRequest.h"
#include "hazelcast/client/collection/CollectionCompareAndRemoveRequest.h"
#include "hazelcast/client/collection/CollectionRemoveRequest.h"
#include "hazelcast/client/collection/CollectionClearRequest.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        /**
        * Concurrent, distributed , client implementation of std::list
        *
        * @param <E> item type
        */
        template<typename E>
        class HAZELCAST_API IList : public DistributedObject {
            friend class HazelcastClient;

        public:

            /**
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            *  @param listener that will be added
            *  @param includeValue bool value representing value should be included in ItemEvent or not.
            *  @returns registrationId that can be used to remove item listener
            */
            std::string addItemListener(ItemListener<E>& listener, bool includeValue) {
                collection::CollectionAddListenerRequest *request = new collection::CollectionAddListenerRequest(getName(), getServiceName(), includeValue);
                impl::ItemEventHandler<E> *entryEventHandler = new impl::ItemEventHandler<E>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return listen(request, entryEventHandler);
            };

            /**
            * Removes the specified item listener.
            * Returns false if the specified listener is not added before.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeItemListener(const std::string& registrationId) {
                collection::CollectionRemoveListenerRequest *request = new collection::CollectionRemoveListenerRequest(getName(), getServiceName(), registrationId);
                return stopListening(request, registrationId);
            };

            /**
            *
            * @return size of the distributed list
            */
            int size() {
                collection::CollectionSizeRequest *request = new collection::CollectionSizeRequest(getName(), getServiceName());
                boost::shared_ptr<int> s = invoke<int>(request, partitionId);
                return *s;
            };

            /**
            *
            * @return true if empty
            */
            bool isEmpty() {
                return size() == 0;
            };

            /**
            *
            * @param element
            * @returns true if list contains element
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool contains(const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                std::vector<serialization::pimpl::Data> valueSet;
                valueSet.push_back(valueData);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), valueSet);
                return *(invoke<bool>(request, partitionId));
            };

            /**
            *
            * @returns all elements as std::vector
            */
            std::vector<E> toArray() {
                collection::CollectionGetAllRequest *request = new collection::CollectionGetAllRequest(getName(), getServiceName());
                boost::shared_ptr<impl::SerializableCollection> result = invoke<impl::SerializableCollection>(request, partitionId);
                const std::vector<serialization::pimpl::Data *>& collection = result->getCollection();
                int size = collection.size();
                std::vector<E> set(size);
                for (int i = 0; i < size; ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*((collection[i])));
                    set[i] = *e;
                }
                return set;
            };

            /**
            *
            * @param element
            * @return true if element is added successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool add(const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                collection::CollectionAddRequest *request = new collection::CollectionAddRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            *
            * @param element
            * @return true if element is removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool remove(const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                collection::CollectionRemoveRequest *request = new collection::CollectionRemoveRequest(getName(), getServiceName(), valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            *
            * @param elements std::vector<E>
            * @return true if this list contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                collection::CollectionContainsRequest *request = new collection::CollectionContainsRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                collection::CollectionAddAllRequest *request = new collection::CollectionAddAllRequest(getName(), getServiceName(), dataCollection);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            * Adds elements in vector to the list with given order.
            * Starts adding elements from given index,
            * and shifts others to the right.
            *
            * @param index start point of insterting given elements
            * @param elements vector of elements that will be added to list
            * @return true if list elements are added.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            bool addAll(int index, const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                list::ListAddAllRequest *request = new list::ListAddAllRequest(getName(), getServiceName(), dataCollection, index);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, false);
                boost::shared_ptr<bool> success = invoke<bool>(request, partitionId);
                return *success;
            };

            /**
            *
            * Removes the elements from this list that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                collection::CollectionCompareAndRemoveRequest *request = new collection::CollectionCompareAndRemoveRequest(getName(), getServiceName(), dataCollection, true);
                return *(invoke<bool>(request, partitionId));
            };

            /**
            * Removes all elements from list.
            */
            void clear() {
                collection::CollectionClearRequest *request = new collection::CollectionClearRequest(getName(), getServiceName());
                invoke<serialization::pimpl::Void>(request, partitionId);
            };

            /**
            * You can check if element is available by
            *
            *      boost::shared_ptr<int> e = list.get(5);
            *      if(e.get() != NULL )
            *          //......;
            *
            * @param index
            * @return element in given index. If not available returns empty constructed shared_ptr.
            * @throws IndexOutOfBoundsException if the index is out of range.
            *
            */
            boost::shared_ptr<E> get(int index) {
                list::ListGetRequest *request = new list::ListGetRequest(getName(), getServiceName(), index);
                return invoke<E>(request, partitionId);
            };

            /**
            * Replaced the element in the given index. And returns element if there were entry before inserting.
            *
            * @param index insert position
            * @param element to be inserted.
            * @return oldElement in given index.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            boost::shared_ptr<E> set(int index, const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                list::ListSetRequest *request = new list::ListSetRequest(getName(), getServiceName(), valueData, index);
                return invoke<E>(request, partitionId);
            };

            /**
            * Adds the element to the given index. Shifts others to the right.
            *
            * @param index insert position
            * @param element to be inserted.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            void add(int index, const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                list::ListAddRequest *request = new list::ListAddRequest(getName(), getServiceName(), valueData, index);
                invoke<serialization::pimpl::Void>(request, partitionId);
            };

            /**
            *
            * @param index
            * @return element in given index.  If not available returns empty constructed shared_ptr.
            * @see get
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            boost::shared_ptr<E> remove(int index) {
                list::ListRemoveRequest *request = new list::ListRemoveRequest(getName(), getServiceName(), index);
                return invoke<E>(request, partitionId);
            };

            /**
            *
            * @param element that will be searched
            * @return index of first occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int indexOf(const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                list::ListIndexOfRequest *request = new list::ListIndexOfRequest(getName(), getServiceName(), valueData, false);
                boost::shared_ptr<int> i = invoke<int>(request, partitionId);
                return *i;
            };

            /**
            * @param element that will be searched
            * @return index of last occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int lastIndexOf(const E& element) {
                serialization::pimpl::Data valueData = toData(element);
                list::ListIndexOfRequest *request = new list::ListIndexOfRequest(getName(), getServiceName(), valueData, true);
                boost::shared_ptr<int> i = invoke<int>(request, partitionId);
                return *i;
            };

            /**
            *
            * @return the sublist as vector between given indexes.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            std::vector<E> subList(int fromIndex, int toIndex) {
                list::ListSubRequest *request = new list::ListSubRequest(getName(), getServiceName(), fromIndex, toIndex);
                boost::shared_ptr<impl::SerializableCollection> result = invoke<impl::SerializableCollection>(request, partitionId);
                const std::vector<serialization::pimpl::Data *>& collection = result->getCollection();
                int size = collection.size();
                std::vector<E> set(toIndex - fromIndex);
                for (int i = 0; i < size; ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*(collection[i]));
                    set[i] = *e;
                }
                return set;
            };

        private:
            template<typename T>
            const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T>& elements) {
                int size = elements.size();
                std::vector<serialization::pimpl::Data> dataCollection(size);
                for (int i = 0; i < size; ++i) {
                    dataCollection[i] = toData(elements[i]);
                }
                return dataCollection;
            };

            template<typename T>
            serialization::pimpl::Data toData(const T& object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename T>
            boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            IList(const std::string& instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().template toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            };

            void onDestroy() {
            };

            int partitionId;
        };
    }
}

#endif /* HAZELCAST_ILIST */

