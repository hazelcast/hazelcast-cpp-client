/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include <mutex>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace util {

/**
 * This is the base synchronized map which works with any provided value and key
 * types.
 * @tparam K The type of the key for the map.
 * @tparam V The type of the value for the map. The shared_ptr<V> is being kept
 * in the map.
 */
template<typename K, typename V, typename Hash = std::hash<K>>
class SynchronizedMap
{
public:
    SynchronizedMap() = default;

    SynchronizedMap(const SynchronizedMap<K, V>& rhs) { *this = rhs; }

    void operator=(const SynchronizedMap<K, V>& rhs)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::lock_guard<std::mutex> lgRhs(rhs.map_lock_);
        internal_map_ = rhs.internal_map_;
    }

    virtual ~SynchronizedMap()
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        internal_map_.clear();
    }

    bool contains_key(const K& key) const
    {
        std::lock_guard<std::mutex> guard(map_lock_);
        return internal_map_.find(key) != internal_map_.end();
    }

    /**
     *
     * @return the previous value associated with the specified key,
     *         or <tt>null</tt> if there was no mapping for the key
     */
    std::shared_ptr<V> put_if_absent(const K& key, std::shared_ptr<V> value)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        if (internal_map_.count(key) > 0) {
            return internal_map_[key];
        } else {
            internal_map_[key] = value;
            return nullptr;
        }
    }

    /**
     *
     * @return the previous value associated with the specified key,
     *         or <tt>null</tt> if there was no mapping for the key
     */
    std::shared_ptr<V> put(const K& key, std::shared_ptr<V> value)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::shared_ptr<V> returnValue;
        auto foundIter = internal_map_.find(key);
        if (foundIter != internal_map_.end()) {
            returnValue = foundIter->second;
        }
        internal_map_[key] = value;
        return returnValue;
    }

    /**
     * Returns the value to which the specified key is mapped,
     * or {@code null} if this map contains no mapping for the key.
     *
     */
    std::shared_ptr<V> get(const K& key)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        auto foundIter = internal_map_.find(key);
        if (foundIter != internal_map_.end()) {
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
    std::shared_ptr<V> remove(const K& key)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        auto foundIter = internal_map_.find(key);
        if (foundIter != internal_map_.end()) {
            std::shared_ptr<V> v = foundIter->second;
            internal_map_.erase(foundIter);
            return v;
        }

        return nullptr;
    }

    bool remove(const K& key, const std::shared_ptr<V>& value)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        auto foundIter = internal_map_.find(key);
        if (foundIter != internal_map_.end()) {
            auto& foundValue = foundIter->second;
            if (!value || !foundValue) {
                if (value == foundValue) {
                    internal_map_.erase(foundIter);
                    return true;
                }
            }
            if (value == foundValue || *value == *foundValue) {
                internal_map_.erase(foundIter);
                return true;
            }
        }

        return false;
    }

    std::vector<std::pair<K, std::shared_ptr<V>>> entry_set()
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::vector<std::pair<K, std::shared_ptr<V>>> entries;
        entries.reserve(internal_map_.size());
        for (const auto& v : internal_map_) {
            entries.emplace_back(v.first, v.second);
        }
        return entries;
    }

    std::vector<std::pair<K, std::shared_ptr<V>>> clear()
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::vector<std::pair<K, std::shared_ptr<V>>> entries;
        entries.reserve(internal_map_.size());
        for (const auto& v : internal_map_) {
            entries.emplace_back(v.first, v.second);
        }
        internal_map_.clear();
        return entries;
    }

    std::vector<std::shared_ptr<V>> values()
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::vector<std::shared_ptr<V>> valueArray;
        for (const auto& v : internal_map_) {
            valueArray.emplace_back(v.second);
        }
        return valueArray;
    }

    std::vector<K> keys()
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        std::vector<K> keysArray;
        for (const auto& v : internal_map_) {
            keysArray.emplace_back(v.first);
        }
        return keysArray;
    }

    /**
     *
     * @param key The key of the entry to be inserted
     * @param new_value The new value to be inserted for the key
     * @return A pair of value and bool. The second part of the pair is true if
     * the insertion of the new values actually happened, i.e. there was no
     * entry associated with the key, otherwise it is set to false. The first
     * part of the pair is existing value or the newly inserted value.
     */
    std::pair<std::shared_ptr<V>, bool> get_or_put_if_absent(
      const K& key,
      const std::shared_ptr<V>& new_value)
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        auto result = internal_map_.emplace(key, new_value);
        return { result.first->second, result.second };
    }

    virtual size_t size() const
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        return internal_map_.size();
    }

    std::unique_ptr<std::pair<K, std::shared_ptr<V>>> get_entry(
      size_t index) const
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        if (index >= internal_map_.size()) {
            return std::unique_ptr<std::pair<K, std::shared_ptr<V>>>();
        }
        auto it = internal_map_.begin();
        for (size_t i = 0; i < index; ++i) {
            ++it;
        }
        return std::unique_ptr<std::pair<K, std::shared_ptr<V>>>(
          new std::pair<K, std::shared_ptr<V>>(it->first, it->second));
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lg(map_lock_);
        return internal_map_.empty();
    }

private:
    std::unordered_map<K, std::shared_ptr<V>, Hash> internal_map_;
    mutable std::mutex map_lock_;
};
} // namespace util
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
