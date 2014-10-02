#ifndef HAZELCAST_ILIST
#define HAZELCAST_ILIST

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/proxy/IListImpl.h"
#include <stdexcept>


namespace hazelcast {
    namespace client {

        /**
        * Concurrent, distributed , client implementation of std::list
        *
        * @param <E> item type
        */
        template<typename E>
        class HAZELCAST_API IList : public proxy::IListImpl {
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
                impl::ItemEventHandler<E> *entryEventHandler = new impl::ItemEventHandler<E>(getName(), context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return proxy::IListImpl::addItemListener(entryEventHandler, includeValue);
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
                return proxy::IListImpl::removeItemListener(registrationId);
            }

            /**
            *
            * @return size of the distributed list
            */
            int size() {
                return proxy::IListImpl::size();
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
                return proxy::IListImpl::contains(toData(element));
            }

            /**
            *
            * @returns all elements as std::vector
            */
            std::vector<E> toArray() {
                return toObjectCollection<E>(proxy::IListImpl::toArray());
            }

            /**
            *
            * @param element
            * @return true if element is added successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool add(const E& element) {
                return proxy::IListImpl::add(toData(element));
            }

            /**
            *
            * @param element
            * @return true if element is removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool remove(const E& element) {
                return proxy::IListImpl::remove(toData(element));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this list contains all elements given in vector.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsAll(const std::vector<E>& elements) {
                return proxy::IListImpl::containsAll(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool addAll(const std::vector<E>& elements) {
                return proxy::IListImpl::addAll(toDataCollection(elements));
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
                return proxy::IListImpl::addAll(index, toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool removeAll(const std::vector<E>& elements) {
                return proxy::IListImpl::removeAll(toDataCollection(elements));
            }

            /**
            *
            * Removes the elements from this list that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool retainAll(const std::vector<E>& elements) {
                return proxy::IListImpl::retainAll(toDataCollection(elements));
            }

            /**
            * Removes all elements from list.
            */
            void clear() {
                proxy::IListImpl::clear();
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
                return toObject<E>(proxy::IListImpl::get(index));
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
                return toObject<E>(proxy::IListImpl::set(index, toData(element)));
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
                proxy::IListImpl::add(index, toData(element));
            }

            /**
            *
            * @param index
            * @return element in given index.  If not available returns empty constructed shared_ptr.
            * @see get
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            boost::shared_ptr<E> remove(int index) {
                return toObject<E>(proxy::IListImpl::remove(index));
            }

            /**
            *
            * @param element that will be searched
            * @return index of first occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int indexOf(const E& element) {
                return proxy::IListImpl::indexOf(toData(element));
            }

            /**
            * @param element that will be searched
            * @return index of last occurrence of given element in the list.
            * Returns -1 if element is not in the list.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            int lastIndexOf(const E& element) {
                return proxy::IListImpl::lastIndexOf(toData(element));
            }

            /**
            *
            * @return the sublist as vector between given indexes.
            * @throws IndexOutOfBoundsException if the index is out of range.
            */
            std::vector<E> subList(int fromIndex, int toIndex) {
                return toObjectCollection<E>(proxy::IListImpl::subList(fromIndex, toIndex));
            }

        private:
            IList(const std::string& instanceName, spi::ClientContext *context)
            : proxy::IListImpl(instanceName, context) {
            }
        };
    }
}

#endif /* HAZELCAST_ILIST */

