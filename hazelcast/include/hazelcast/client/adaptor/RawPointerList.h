/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERLIST_H_
#define HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERLIST_H_

#include "hazelcast/client/IList.h"
#include "hazelcast/client/adaptor/MapEntryView.h"
#include "hazelcast/client/impl/DataArrayImpl.h"
#include "hazelcast/client/impl/EntryArrayImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * Concurrent, distributed , client implementation of std::list
            *
            * @param <T> item type
            */
            template<typename T>
            class RawPointerList {
            public:
                RawPointerList(IList<T> &listToBeAdopted): list(listToBeAdopted), serializationService(
                        list.context->getSerializationService()) {
                }

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
                std::string addItemListener(ItemListener<T> &listener, bool includeValue) {
                    return list.addItemListener(listener, includeValue);
                }

                /**
                * Removes the specified item listener.
                * Returns false if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeItemListener(const std::string &registrationId) {
                    return list.removeItemListener(registrationId);
                }

                /**
                *
                * @return size of the distributed list
                */
                int size() {
                    return list.size();
                }

                /**
                *
                * @return true if empty
                */
                bool isEmpty() {
                    return list.isEmpty();
                }

                /**
                *
                * @param element
                * @returns true if list contains element
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool contains(const T &element) {
                    return list.contains(element);
                }

                /**
                *
                * @returns all elements in the list
                */
                std::auto_ptr<DataArray<T> > toArray() {
                    return std::auto_ptr<DataArray<T> >(new impl::DataArrayImpl<T>(list.toArrayData(), serializationService));
                }

                /**
                *
                * @param element
                * @return true if element is added successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool add(const T &element) {
                    return list.add(element);
                }

                /**
                *
                * @param element
                * @return true if element is removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool remove(const T &element) {
                    return list.remove(element);
                }

                /**
                *
                * @param elements Items to look for in the list
                * @return true if this list contains all elements given in vector.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool containsAll(const std::vector<T> &elements) {
                    return list.containsAll(elements);
                }

                /**
                *
                * @param elements Items to be added to the list
                * @return true if all elements given in vector can be added to list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool addAll(const std::vector<T> &elements) {
                    return list.addAll(elements);
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
                bool addAll(int index, const std::vector<T> &elements) {
                    return list.addAll(index, elements);
                }

                /**
                *
                * @param elements Items to be removed from the list
                * @return true if all elements are removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool removeAll(const std::vector<T> &elements) {
                    return list.removeAll(elements);
                }

                /**
                *
                * Removes the elements from this list that are not available in given "elements" vector
                * @param elements Items to retain in the list
                * @return true if operation is successful.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool retainAll(const std::vector<T> &elements) {
                    return list.retainAll(elements);
                }

                /**
                * Removes all elements from list.
                */
                void clear() {
                    list.clear();
                }

                /**
                * You can check if element is available by
                *
                *      std::auto_ptr<int> e = list.get(5);
                *      if(e.get() != NULL )
                *          //......;
                *
                * @param index
                * @return element in given index. If not available returns empty constructed auto_ptr.
                * @throws IndexOutOfBoundsException if the index is out of range.
                *
                */
                std::auto_ptr<T> get(int index) {
                    return serializationService.toObject<T>(list.getData(index).get());
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
                std::auto_ptr<T> set(int index, const T &element) {
                    return serializationService.toObject<T>(list.setData(index, serializationService.toData<T>(&element)).get());
                }

                /**
                * Adds the element to the given index. Shifts others to the right.
                *
                * @param index insert position
                * @param element to be inserted.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                void add(int index, const T &element) {
                    list.add(index, element);
                }

                /**
                *
                * @param index
                * @return element in given index.  If not available returns empty constructed auto_ptr.
                * @see get
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                std::auto_ptr<T> remove(int index) {
                    return serializationService.toObject<T>(list.removeData(index).get());
                }

                /**
                *
                * @param element that will be searched
                * @return index of first occurrence of given element in the list.
                * Returns -1 if element is not in the list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                int indexOf(const T &element) {
                    return list.indexOf(element);
                }

                /**
                * @param element that will be searched
                * @return index of last occurrence of given element in the list.
                * Returns -1 if element is not in the list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                int lastIndexOf(const T &element) {
                    return list.lastIndexOf(element);
                }

                /**
                *
                * @return the sublist between given indexes.
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                std::auto_ptr<DataArray<T> > subList(int fromIndex, int toIndex) {
                    return std::auto_ptr<DataArray<T> >(new impl::DataArrayImpl<T>(list.subListData(fromIndex, toIndex), serializationService));
                }

            private:
                IList<T> &list;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERLIST_H_ */

