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

#include <boost/thread/shared_mutex.hpp>

#include "hazelcast/util/HazelcastDll.h"


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
        template <typename K, typename V, typename Hash = std::hash<K>>
        class SynchronizedMap {
        public:
            SynchronizedMap() = default;

            SynchronizedMap(const SynchronizedMap<K, V> &rhs) {
                *this = rhs;
            }

            void operator=(const SynchronizedMap<K, V> &rhs) {
                boost::unique_lock<boost::shared_mutex> g1(mapLock);
                boost::unique_lock<boost::shared_mutex> g2(rhs.mapLock);
                internalMap = rhs.internalMap;
            }

            virtual ~SynchronizedMap() {
                boost::unique_lock<boost::shared_mutex> g(mapLock);
                internalMap.clear();
            }

            bool containsKey(const K &key) const {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                return internalMap.find(key) != internalMap.end();
            }

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            std::shared_ptr<V> putIfAbsent(const K &key, std::shared_ptr<V> value) {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                auto found = internalMap.find(key);
                if (found != internalMap.end()) {
                    return found->second;
                } else {
                    // upgrade to write lock
                    boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
                    found = internalMap.find(key);
                    if (found != internalMap.end()) {
                        return found->second;
                    }
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
                boost::unique_lock<boost::shared_mutex> write_lock(mapLock);
                std::shared_ptr<V> returnValue;
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    returnValue = foundIter->second;
                    foundIter->second = value;
                } else {
                    internalMap[key] = value;
                }
                return returnValue;
            }

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            std::shared_ptr<V> get(const K &key) {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
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
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    // upgrade to write lock
                    boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
                    foundIter = internalMap.find(key);
                    std::shared_ptr<V> v = foundIter->second;
                    if (foundIter != internalMap.end()) {
                        internalMap.erase(key);
                    }
                    return v;
                }

                return nullptr;
            }

            bool remove(const K &key, const std::shared_ptr<V> &value) {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                auto foundIter = internalMap.find(key);
                if (foundIter != internalMap.end()) {
                    auto &foundValue = foundIter->second;
                    if (!value || !foundValue) {
                        if (value == foundValue) {
                            // upgrade to write lock
                            boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
                            return internalMap.erase(key) > 0;
                        }
                    }
                    if (value == foundValue || *value == *foundValue) {
                        // upgrade to write lock
                        boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
                        return internalMap.erase(key) > 0;
                    }
                }

                return false;
            }

            std::vector<std::pair<K, std::shared_ptr<V> > > entrySet() {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                std::vector<std::pair<K, std::shared_ptr<V> > > entries;
                entries.reserve(internalMap.size());
                for (const auto &v : internalMap) {
                    entries.emplace_back(v.first, v.second);
                }
                return entries;
            }

            std::vector<std::pair<K, std::shared_ptr<V> > > clear() {
                boost::unique_lock<boost::shared_mutex> write_lock(mapLock);
                std::vector<std::pair<K, std::shared_ptr<V> > > entries;
                entries.reserve(internalMap.size());
                for (const auto &v : internalMap) {
                    entries.emplace_back(v.first, v.second);
                }
                internalMap.clear();
                return entries;
            }

            std::vector<std::shared_ptr<V> > values() {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                std::vector<std::shared_ptr<V> > valueArray;
                for (const auto &v : internalMap) {
                    valueArray.emplace_back(v.second);
                }
                return valueArray;
            }

            std::vector<K> keys() {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                std::vector<K> keysArray;
                for (const auto &v : internalMap) {
                    keysArray.emplace_back(v.first);
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
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                return internalMap.size();
            }

            std::unique_ptr<std::pair<K, std::shared_ptr<V> > > getEntry(size_t index) const {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                if (index < 0 || index >= internalMap.size()) {
                    return std::unique_ptr<std::pair<K, std::shared_ptr<V> > >();
                }
                auto it = internalMap.begin();
                for (size_t i = 0; i < index; ++i) {
                    ++it;
                }
                return std::unique_ptr<std::pair<K, std::shared_ptr<V> > >(
                        new std::pair<K, std::shared_ptr<V> >(it->first, it->second));
            }

            bool empty() const {
                boost::upgrade_lock<boost::shared_mutex> read_lock(mapLock);
                return internalMap.empty();
            }

        private:
            std::unordered_map<K, std::shared_ptr<V>, Hash> internalMap;
            mutable boost::shared_mutex mapLock;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif





