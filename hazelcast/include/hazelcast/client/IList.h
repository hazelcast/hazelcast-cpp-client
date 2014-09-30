#ifndef HAZELCAST_ILIST
#define HAZELCAST_ILIST

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/pimpl/IListImpl.h"
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
                impl::ItemEventHandler<E> *entryEventHandler = new impl::ItemEventHandler<E>(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return impl->addItemListener(entryEventHandler, includeValue);
            }

            /**
            * Removes the specified item listener.
            * Returns false if the specified listener is not added before.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeItemListener(const std::string& registrationId) {
                return impl->removeItemListener(registrationId);
            }

            /**
            *
            * @return size of the distributed list
            */
            int size() {
                return impl->size();
            }

            /**
            *
            * @return true if empty
            */
            bool isEmpty() {
                return size() == 0;
            }

            /**
            *
            * @param element
            * @returns true if list contains element
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool contains(const E& element) {
                return impl->contains(toData(element));
            }

            /**
            *
            * @returns all elements as std::vector
            */
            std::vector<E> toArray() {
                std::vector<serialization::pimpl::Data *> collection = impl->toArray();
                int size = collection.size();
                std::vector<E> set(size);
                for (int i = 0; i < size; ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*((collection[i])));
                    set[i] = *e;
                }
                return set;
            }

            /**
            *
            * @param element
            * @return true if element is added successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool add(const E& element) {
                return impl->add(toData(element));
            }

            /**
            *
            * @param element
            * @return true if element is removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool remove(const E& element) {
                return impl->remove(toData(element));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this list contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                return impl->containsAll(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                return impl->addAll(toDataCollection(elements));
            }

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
                return impl->addAll(index, toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                return impl->removeAll(toDataCollection(elements));
            }

            /**
            *
            * Removes the elements from this list that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                return impl->retainAll(toDataCollection(elements));
            }

            /**
            * Removes all elements from list.
            */
            void clear() {
                impl->clear();
            }

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
                return toObject<E>(impl->get(index));
            }

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
                return toObject<E>(impl->set(index, toData(element)));
            }

            /**
            * Adds the element to the given index. Shifts others to the right.
            *
            * @param index insert position
            * @param element to be inserted.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            void add(int index, const E& element) {
                impl->add(index, toData(element));
            }

            /**
            *
            * @param index
            * @return element in given index.  If not available returns empty constructed shared_ptr.
            * @see get
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            boost::shared_ptr<E> remove(int index) {
                return toObject<E>(impl->remove(index));
            }

            /**
            *
            * @param element that will be searched
            * @return index of first occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int indexOf(const E& element) {
                return impl->indexOf(toData(element));
            }

            /**
            * @param element that will be searched
            * @return index of last occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int lastIndexOf(const E& element) {
                return impl->lastIndexOf(toData(element));
            }

            /**
            *
            * @return the sublist as vector between given indexes.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            std::vector<E> subList(int fromIndex, int toIndex) {
                std::vector<serialization::pimpl::Data *> collection = impl->subList(fromIndex, toIndex);
                int size = collection.size();
                std::vector<E> set(toIndex - fromIndex);
                for (int i = 0; i < size; ++i) {
                    boost::shared_ptr<E> e = toObject<E>(*(collection[i]));
                    set[i] = *e;
                }
                return set;
            }

            /**
            * Destructor
            */
            ~IList() {
                delete impl;
            }

        private:
            template<typename T>
            const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T>& elements) {
                int size = elements.size();
                std::vector<serialization::pimpl::Data> dataCollection(size);
                for (int i = 0; i < size; ++i) {
                    dataCollection[i] = toData(elements[i]);
                }
                return dataCollection;
            }

            template<typename T>
            serialization::pimpl::Data toData(const T& object) {
                return getContext().getSerializationService().template toData<T>(&object);
            }

            template<typename T>
            boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                return getContext().getSerializationService().template toObject<T>(data);
            }

            IList(const std::string& instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:listService", instanceName, context)
            , impl(new pimpl::IListImpl(instanceName, context)) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().template toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            void onDestroy() {
            }

            pimpl::IListImpl *impl;
            int partitionId;
        };
    }
}

#endif /* HAZELCAST_ILIST */

