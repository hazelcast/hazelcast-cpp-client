/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once
#include "hazelcast/client/proxy/ISetImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace mixedtype {

            /**
            * Concurrent, distributed client implementation of std::unordered_set.
            *
            */
            class HAZELCAST_API ISet : public proxy::ISetImpl {
                friend class client::impl::HazelcastClientInstanceImpl;;

            public:
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
                * @returns size of the distributed set
                */
                int size();

                /**
                *
                * @returns true if empty
                */
                bool isEmpty();

                /**
                *
                * @param element to be searched
                * @returns true if set contains element
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool contains(const E &element) {
                    return proxy::ISetImpl::contains(toData(element));
                }

                /**
                *
                * @returns all elements as std::vector
                */
                std::vector<TypedData> toArray();

                /**
                *
                * @param element to be added
                * @return true if element is added successfully. If elements was already there returns false.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool add(const E &element) {
                    return proxy::ISetImpl::add(toData(element));
                }

                /**
                *
                * @param element to be removed
                * @return true if element is removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool remove(const E &element) {
                    return proxy::ISetImpl::remove(toData(element));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if this set contains all elements given in vector.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool containsAll(const std::vector<E> &elements) {
                    return proxy::ISetImpl::containsAll(toDataCollection(elements));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if all elements given in vector can be added to set.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool addAll(const std::vector<E> &elements) {
                    std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                    return proxy::ISetImpl::addAll(toDataCollection(elements));
                }

                /**
                *
                * @param elements std::vector<E>
                * @return true if all elements are removed successfully.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool removeAll(const std::vector<E> &elements) {
                    std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                    return proxy::ISetImpl::removeAll(dataCollection);
                }

                /**
                *
                * Removes the elements from this set that are not available in given "elements" vector
                * @param elements std::vector<E>
                * @return true if operation is successful.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename E>
                bool retainAll(const std::vector<E> &elements) {
                    return proxy::ISetImpl::retainAll(toDataCollection(elements));
                }

                /**
                *
                * Removes all elements from set.
                */
                void clear();

            private:
                ISet(const std::string &instanceName, spi::ClientContext *context);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


