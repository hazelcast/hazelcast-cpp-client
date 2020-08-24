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

#include <unordered_set>

#include "hazelcast/util/HazelcastDll.h"
#include <mutex>


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {

        template <typename Key, typename Hash = std::hash<Key>>
        class sync_unordered_set  {
        public:
            std::pair<typename std::unordered_set<Key, Hash>::iterator, bool> insert(const Key& key) {
                std::lock_guard<std::mutex> g(lock_);
                return set_.insert(key);
            }
            typename std::unordered_set<Key, Hash>::size_type erase( const Key& key) {
                std::lock_guard<std::mutex> g(lock_);
                return set_.erase(key);
            }
        private:
            std::unordered_set<Key, Hash> set_;
            std::mutex lock_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif





