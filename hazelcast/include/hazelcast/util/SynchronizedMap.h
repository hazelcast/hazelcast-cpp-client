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
//
// Created by sancar koyunlu on 5/21/13.

#ifndef HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_
#define HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        template <typename K, typename V, typename Comparator  = std::less<K> >
        class SynchronizedMap {
        public:
            SynchronizedMap() {
            }

            SynchronizedMap(const SynchronizedMap<K, V, Comparator> &rhs) {
                util::LockGuard lg(mapLock);
                util::LockGuard lgRhs(rhs.mapLock);
                internalMap = rhs.internalMap;
            }

            virtual ~SynchronizedMap() {
                util::LockGuard lg(mapLock);
                internalMap.clear();
            };

            bool containsKey(const K &key) const {
                util::LockGuard guard(mapLock);
                return internalMap.count(key) > 0;
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            boost::shared_ptr<V> putIfAbsent(const K &key, boost::shared_ptr<V> value) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return boost::shared_ptr<V>();
                }
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            boost::shared_ptr<V> put(const K &key, boost::shared_ptr<V> value) {
                util::LockGuard lg(mapLock);
                boost::shared_ptr<V> returnValue;
                if (internalMap.count(key) > 0) {
                    returnValue = internalMap[key];
                }
                internalMap[key] = value;
                return returnValue;
            };

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            boost::shared_ptr<V> get(const K &key) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return boost::shared_ptr<V>();
            };

            /**
            * Returns the value to which the specified key is mapped,
            * and removes from map
            * or {@code null} if this map contains no mapping for the key.
            *
            */
            boost::shared_ptr<V> remove(const K &key) {
                util::LockGuard lg(mapLock);
                if (internalMap.count(key) > 0) {
                    boost::shared_ptr<V> v = internalMap[key];
                    internalMap.erase(key);
                    return v;
                }
                else
                    return boost::shared_ptr<V>();
            };

            std::vector<std::pair<K, boost::shared_ptr<V> > > entrySet() {
                util::LockGuard lg(mapLock);
                std::vector<std::pair<K, boost::shared_ptr<V> > > entries(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries[i++] = std::pair<K, boost::shared_ptr<V> >(it->first, it->second);
                }
                return entries;
            }

            std::vector<std::pair<K, boost::shared_ptr<V> > > clear() {
                util::LockGuard lg(mapLock);
                std::vector<std::pair<K, boost::shared_ptr<V> > > entries(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries[i++] = std::pair<K, boost::shared_ptr<V> >(it->first, it->second);
                }
                internalMap.clear();
                return entries;
            }

            std::vector<boost::shared_ptr<V> > values() {
                util::LockGuard lg(mapLock);
                std::vector<boost::shared_ptr<V> > valueArray(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    valueArray[i++] = it->second;
                }
                return valueArray;
            }

            std::vector<K> keys() {
                util::LockGuard lg(mapLock);
                std::vector<K> keysArray(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    keysArray[i++] = it->first;
                }
                return keysArray;
            }

            boost::shared_ptr<V> getOrPutIfAbsent(const K &key) {
                boost::shared_ptr<V> value = get(key);
                if (value.get() == NULL) {
                    value.reset(new V());
                    boost::shared_ptr<V> current = putIfAbsent(key, value);
                    value = current.get() == NULL ? value : current;
                }
                return value;
            }

            virtual size_t size() const {
                util::LockGuard lg(mapLock);
                return internalMap.size();
            }

            std::auto_ptr<std::pair<K, boost::shared_ptr<V> > > getEntry(size_t index) const {
                util::LockGuard lg(mapLock);
                if (index < 0 || index >= internalMap.size()) {
                    return std::auto_ptr<std::pair<K, boost::shared_ptr<V> > >();
                }
                typename std::map<K, boost::shared_ptr<V> >::const_iterator it = internalMap.begin();
                for (size_t i = 0; i < index; ++i) {
                    ++it;
                }
                return std::auto_ptr<std::pair<K, boost::shared_ptr<V> > >(
                        new std::pair<K, boost::shared_ptr<V> >(it->first, it->second));
            }
        private:
            std::map<K, boost::shared_ptr<V>, Comparator> internalMap;
            mutable util::Mutex mapLock;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_SYNCHRONIZEDMAP_H_

