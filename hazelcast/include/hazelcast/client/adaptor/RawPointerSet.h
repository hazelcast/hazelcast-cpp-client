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
#ifndef HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERSET_H_
#define HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERSET_H_

#include "hazelcast/client/ISet.h"
#include "hazelcast/client/impl/DataArrayImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * Concurrent, distributed client implementation of std::unordered_set.
            *
            * @tparam T item type
            */
            template<typename T>
            class RawPointerSet {
            public:
                RawPointerSet(ISet<T> &s) : set(s), serializationService(s.context->getSerializationService()) {
                }

                /**
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                *  @param listener to be added
                *  @param includeValue boolean value representing value should be included in incoming ItemEvent or not.
                *  @returns registrationId that can be used to remove item listener
                */
                std::string addItemListener(ItemListener<T> &listener, bool includeValue) {
                    return set.addItemListener(listener, includeValue);
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
                    return set.removeItemListener(registrationId);
                }

                /**
                *
                * @returns size of the distributed set
                */
                int size() {
                    return set.size();
                }

                /**
                *
                * @returns true if empty
                */
                bool isEmpty() {
                    return set.isEmpty();
                }

                /**
                *
                * @param element to be searched
                * @returns true if set contains element
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool contains(const T &element) {
                    return set.contains(element);
                }

                /**
                *
                * @returns all elements as std::vector
                */
                std::auto_ptr<DataArray<T> > toArray() {
                    return std::auto_ptr<DataArray<T> >(new hazelcast::client::impl::DataArrayImpl<T>(set.toArrayData(), serializationService));
                }

                /**
                *
                * @param element to be added
                * @return true if element is added successfully. If elements was already there returns false.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool add(const T &element) {
                    return set.add(element);
                }

                /**
                *
                * @param element to be removed
                * @return true if element is removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool remove(const T &element) {
                    return set.remove(element);
                }

                /**
                *
                * @param elements std::vector<T>
                * @return true if this set contains all elements given in vector.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool containsAll(const std::vector<T> &elements) {
                    return set.containsAll(elements);
                }

                /**
                *
                * @param elements std::vector<T>
                * @return true if all elements given in vector can be added to set.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool addAll(const std::vector<T> &elements) {
                    return set.addAll(elements);
                }

                /**
                *
                * @param elements std::vector<T>
                * @return true if all elements are removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool removeAll(const std::vector<T> &elements) {
                    return set.removeAll(elements);
                }

                /**
                *
                * Removes the elements from this set that are not available in given "elements" vector
                * @param elements std::vector<T>
                * @return true if operation is successful.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool retainAll(const std::vector<T> &elements) {
                    return set.retainAll(elements);
                }

                /**
                *
                * Removes all elements from set.
                */
                void clear() {
                    set.clear();
                }

            private:
                ISet<T> &set;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}

#endif /* HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERSET_H_ */

