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

#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>

#include "hazelcast/util/HazelcastDll.h"
#include <mutex>


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {

        /**
         * This is the base synchronized map which works with any provided value and key types.
         * @tparam K The type of the key for the map.
         * @tparam V The type of the value for the map. The shared_ptr<V> is being kept in the map.
         */
        template <typename K, typename V>
        class SynchronizedMap {
        public:
            SynchronizedMap() = default;

            SynchronizedMap(const SynchronizedMap<K, V> &rhs) {
                *this = rhs;
            }

            void operator=(const SynchronizedMap<K, V> &rhs) {
                std::lock_guard<std::mutex> lg(mapLock);
                std::lock_guard<std::mutex> lgRhs(rhs.mapLock);
                internalMap = rhs.internalMap;
            }

            virtual ~SynchronizedMap() {
                std::lock_guard<std::mutex> lg(mapLock);
                internalMap.clear();
            }

            bool containsKey(const K &key) const {
                std::lock_guard<std::mutex> guard(mapLock);
                return internalMap.find(key) != internalMap.end();
            }

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            std::shared_ptr<V> putIfAbsent(const K &key, std::shared_ptr<V> value) {
                std::lock_guard<std::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return nullptr;
                }
            }

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            std::shared_ptr<V> put(const K &key, std::shared_ptr<V> value) {
                std::lock_guard<std::mutex> lg(mapLock);
                std::shared_ptr<V> returnValue;
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    returnValue = foundIter->second;
                }
                internalMap[key] = value;
                return returnValue;
            }

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            std::shared_ptr<V> get(const K &key) {
                std::lock_guard<std::mutex> lg(mapLock);
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    return foundIter->second;
                }

                return nullptr;
            }

            /**
            * Returns the value to which the specified key is mapped,
            * and removes from map
            * or {@code null} if this map contains no mapping for the key.
            *
            */
            std::shared_ptr<V> remove(const K &key) {
                std::lock_guard<std::mutex> lg(mapLock);
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    std::shared_ptr<V> v = foundIter->second;
                    internalMap.erase(foundIter);
                    return v;
                }

                return nullptr;
            }

            bool remove(const K &key, const std::shared_ptr<V> &value) {
                std::lock_guard<std::mutex> lg(mapLock);
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    auto &foundValue = foundIter->second;
                    if (!value || !foundValue) {
                        if (value == foundValue) {
                            internalMap.erase(foundIter);
                            return true;
                        }
                    }
                    if (*value == *foundValue) {
                        internalMap.erase(foundIter);
                        return true;
                    }
                }

                return false;
            }

            std::vector<std::pair<K, std::shared_ptr<V> > > entrySet() {
                std::lock_guard<std::mutex> lg(mapLock);
                std::vector<std::pair<K, std::shared_ptr<V> > > entries;
                typename std::unordered_map<K, std::shared_ptr<V>>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries.push_back(std::pair<K, std::shared_ptr<V> >(it->first, it->second));
                }
                return entries;
            }

            std::vector<std::pair<K, std::shared_ptr<V> > > clear() {
                std::lock_guard<std::mutex> lg(mapLock);
                std::vector<std::pair<K, std::shared_ptr<V> > > entries;
                typename std::unordered_map<K, std::shared_ptr<V>>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries.push_back(std::pair<K, std::shared_ptr<V> >(it->first, it->second));
                }
                internalMap.clear();
                return entries;
            }

            std::vector<std::shared_ptr<V> > values() {
                std::lock_guard<std::mutex> lg(mapLock);
                std::vector<std::shared_ptr<V> > valueArray;
                typename std::unordered_map<K, std::shared_ptr<V>>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    valueArray.push_back(it->second);
                }
                return valueArray;
            }

            std::vector<K> keys() {
                std::lock_guard<std::mutex> lg(mapLock);
                std::vector<K> keysArray;
                typename std::unordered_map<K, std::shared_ptr<V>>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    keysArray.push_back(it->first);
                }
                return keysArray;
            }

            std::shared_ptr<V> getOrPutIfAbsent(const K &key) {
                std::shared_ptr<V> value = get(key);
                if (!value) {
                    value.reset(new V());
                    std::shared_ptr<V> current = putIfAbsent(key, value);
                    value = !current ? value : current;
                }
                return value;
            }

            virtual size_t size() const {
                std::lock_guard<std::mutex> lg(mapLock);
                return internalMap.size();
            }

            std::unique_ptr<std::pair<K, std::shared_ptr<V> > > getEntry(size_t index) const {
                std::lock_guard<std::mutex> lg(mapLock);
                if (index < 0 || index >= internalMap.size()) {
                    return std::unique_ptr<std::pair<K, std::shared_ptr<V> > >();
                }
                typename std::unordered_map<K, std::shared_ptr<V> >::const_iterator it = internalMap.begin();
                for (size_t i = 0; i < index; ++i) {
                    ++it;
                }
                return std::unique_ptr<std::pair<K, std::shared_ptr<V> > >(
                        new std::pair<K, std::shared_ptr<V> >(it->first, it->second));
            }
        private:
            std::unordered_map<K, std::shared_ptr<V>> internalMap;
            mutable std::mutex mapLock;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif





