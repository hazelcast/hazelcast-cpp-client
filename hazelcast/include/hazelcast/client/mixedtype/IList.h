/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_MIXEDTYPE_ILIST_H_
#define HAZELCAST_CLIENT_MIXEDTYPE_ILIST_H_

#include <vector>
#include <string>

#include "hazelcast/client/proxy/IListImpl.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/ItemListener.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            /**
            * Concurrent, distributed , client implementation of std::list
            *
            */
            class HAZELCAST_API IList : public proxy::IListImpl {
                friend class client::impl::HazelcastClientInstanceImpl;

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
                std::string addItemListener(MixedItemListener &listener, bool includeValue);

                /**
                * Removes the specified item listener.
                * Returns false if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeItemListener(const std::string &registrationId);

                /**
                *
                * @return size of the distributed list
                */
                int size();

                /**
                *
                * @return true if empty
                */
                bool isEmpty();

                /**
                *
                * @param element
                * @returns true if list contains element
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool contains(const E &element) {
                    return proxy::IListImpl::contains(toData<E>(element));
                }

                /**
                *
                * @returns all elements as std::vector
                */
                std::vector<TypedData> toArray();

                /**
                *
                * @param element
                * @return true if element is added successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool add(const E &element) {
                    return proxy::IListImpl::add(toData<E>(element));
                }

                /**
                *
                * @param element
                * @return true if element is removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool remove(const E &element) {
                    return proxy::IListImpl::remove(toData<E>(element));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if this list contains all elements given in vector.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool containsAll(const std::vector<E> &elements) {
                    return proxy::IListImpl::containsAll(toDataCollection<E>(elements));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if all elements given in vector can be added to list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool addAll(const std::vector<E> &elements) {
                    return proxy::IListImpl::addAll(toDataCollection<E>(elements));
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
                template <typename E>
                bool addAll(int index, const std::vector<E> &elements) {
                    return proxy::IListImpl::addAll(index, toDataCollection<E>(elements));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if all elements are removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool removeAll(const std::vector<E> &elements) {
                    return proxy::IListImpl::removeAll(toDataCollection<E>(elements));
                }

                /**
                *
                * Removes the elements from this list that are not available in given "elements" vector
                * @param elements std::vector<E>
                * @return true if operation is successful.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool retainAll(const std::vector<E> &elements) {
                    return proxy::IListImpl::retainAll(toDataCollection<E>(elements));
                }

                /**
                * Removes all elements from list.
                */
                void clear();

                /**
                * You can check if element is available by
                *
                *      std::auto_ptr<int> e = list.get(5).get<int>();
                *      if(e.get() != NULL )
                *          //......;
                *
                * @param index
                * @return element in given index. If not available returns empty constructed shared_ptr.
                * @throws IndexOutOfBoundsException if the index is out of range.
                *
                */
                TypedData get(int index);

                /**
                * Replaced the element in the given index. And returns element if there were entry before inserting.
                *
                * @param index insert position
                * @param element to be inserted.
                * @return oldElement in given index.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                template <typename E>
                TypedData set(int index, const E &element) {
                    return TypedData(proxy::IListImpl::setData(index, toData(element)), getContext().getSerializationService());
                }

                /**
                * Adds the element to the given index. Shifts others to the right.
                *
                * @param index insert position
                * @param element to be inserted.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                template <typename E>
                void add(int index, const E &element) {
                    proxy::IListImpl::add(index, toData(element));
                }

                /**
                *
                * @param index
                * @return element in given index.  If not available returns empty constructed shared_ptr.
                * @see get
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                TypedData remove(int index);

                /**
                *
                * @param element that will be searched
                * @return index of first occurrence of given element in the list.
                * Returns -1 if element is not in the list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                int indexOf(const E &element) {
                    return proxy::IListImpl::indexOf(toData(element));
                }

                /**
                * @param element that will be searched
                * @return index of last occurrence of given element in the list.
                * Returns -1 if element is not in the list.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                int lastIndexOf(const E &element) {
                    return proxy::IListImpl::lastIndexOf(toData(element));
                }

                /**
                *
                * @return the sublist as vector between given indexes.
                * @throws IndexOutOfBoundsException if the index is out of range.
                */
                std::vector<TypedData> subList(int fromIndex, int toIndex);

            private:
                IList(const std::string &instanceName, spi::ClientContext *context);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MIXEDTYPE_ILIST_H_ */

