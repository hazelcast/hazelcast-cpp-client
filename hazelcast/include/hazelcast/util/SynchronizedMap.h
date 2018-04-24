/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 5/21/13.

#ifndef HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_
#define HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_

#include <map>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {

        /**
         * This is the base synchronized map which works with any provided value and key types. E.g. you can keep just
         * raw pointers as values and no memory management of entries is done since values are pointers.
         * @tparam K The type of the key for the map.
         * @tparam V The type of the value for the map.
         * @tparam Comparator The comparator to be used for the key type.
         */
        template <typename K, typename V, typename Comparator  = std::less<K> >
        class SynchronizedValueMap {
        public:
            SynchronizedValueMap() {
            }

            SynchronizedValueMap(const SynchronizedValueMap<K, V, Comparator> &rhs) {
                util::LockGuard lg(mapLock);
                util::LockGuard lgRhs(rhs.mapLock);
                internalMap = rhs.internalMap;
            }

            virtual ~SynchronizedValueMap() {
                util::LockGuard lg(mapLock);
                internalMap.clear();
            }

            bool containsKey(const K &key) const {
                util::LockGuard guard(mapLock);
                return internalMap.count(key) > 0;
            }

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>V()</tt> if there was no mapping for the key
             */
            V putIfAbsent(const K &key, const V &value) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return V();
                }
            }

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>V()</tt> if there was no mapping for the key
             */
            V put(const K &key, const V &value) {
                util::LockGuard lg(mapLock);
                V returnValue;
                if (internalMap.count(key) > 0) {
                    returnValue = internalMap[key];
                }
                internalMap[key] = value;
                return returnValue;
            }

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code V()} if this map contains no mapping for the key.
             *
             */
            V get(const K &key) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else {
                    return V();
                }
            }

            /**
            * Returns the value to which the specified key is mapped,
            * and removes from map
            * or {@code V()} if this map contains no mapping for the key.
            *
            */
            V remove(const K &key) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0) {
                    V v = internalMap[key];
                    internalMap.erase(key);
                    return v;
                }
                else
                    return V();
            };

            bool remove(const K &key, const V &value) {
                util::LockGuard lg(mapLock);
                for (typename InternalMap::iterator it = internalMap.find(key);
                     it != internalMap.end(); ++it) {
                    if (it->second == value) {
                        internalMap.erase(it);
                        return true;
                    }
                }
                return false;
            }

            std::vector<std::pair<K, V> > entrySet() {
                util::LockGuard lg(mapLock);
                std::vector<std::pair<K, V> > entries;
                BOOST_FOREACH(const typename InternalMap::value_type &entry , internalMap) {
                                entries.push_back(entry);
                            }
                return entries;
            }

            std::vector<std::pair<K, V> > clear() {
                util::LockGuard lg(mapLock);
                std::vector<std::pair<K, V> > entries;
                BOOST_FOREACH(const typename InternalMap::value_type &entry , internalMap) {
                                entries.push_back(entry);
                            }
                internalMap.clear();
                return entries;
            }

            std::vector<V> values() {
                util::LockGuard lg(mapLock);
                std::vector<V> valueArray;
                BOOST_FOREACH(const typename InternalMap::value_type &entry , internalMap) {
                                valueArray.push_back(entry.second);
                            }
                return valueArray;
            }

            std::vector<K> keys() {
                util::LockGuard lg(mapLock);
                std::vector<K> keysArray;
                BOOST_FOREACH(const typename InternalMap::value_type &entry , internalMap) {
                    keysArray.push_back(entry.first);
                }
                return keysArray;
            }

            virtual size_t size() const {
                util::LockGuard lg(mapLock);
                return internalMap.size();
            }

            std::auto_ptr<std::pair<K, V> > getEntry(size_t index) const {
                util::LockGuard lg(mapLock);
                if (index < 0 || index >= internalMap.size()) {
                    return std::auto_ptr<std::pair<K, V> >();
                }
                typename InternalMap::const_iterator it = internalMap.begin();
                for (size_t i = 0; i < index; ++i) {
                    ++it;
                }
                return std::auto_ptr<std::pair<K, V> >(new std::pair<K, V>(it->first, it->second));
            }
        protected:
            typedef std::map<K, V, Comparator> InternalMap;

            InternalMap internalMap;
            mutable util::Mutex mapLock;
        };

        template <typename K, typename V, typename Comparator  = std::less<K> >
        class SynchronizedMap : public SynchronizedValueMap<K, boost::shared_ptr<V>, Comparator> {
        public:
            SynchronizedMap() {}

            SynchronizedMap(const SynchronizedValueMap<K, V, Comparator> &rhs)
                    : SynchronizedValueMap<K, boost::shared_ptr<V>, Comparator>(rhs) {}

            boost::shared_ptr<V> getOrPutIfAbsent(const K &key) {
                util::LockGuard lg(SynchronizedValueMap<K, boost::shared_ptr<V>, Comparator>::mapLock);
                boost::shared_ptr<V> &currentValue = SynchronizedValueMap<K, boost::shared_ptr<V>, Comparator>::internalMap[key];
                if (currentValue.get()) {
                    return currentValue;
                }
                currentValue.reset(new V());
                return currentValue;
            }
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_

