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
#ifndef HAZELCAST_CLIENT_INTERNAL_ADAPTER_IMAPDATASTRUCTUREADAPTER_H_
#define HAZELCAST_CLIENT_INTERNAL_ADAPTER_IMAPDATASTRUCTUREADAPTER_H_

#include "hazelcast/client/internal/adapter/DataStructureAdapter.h"
#include "hazelcast/client/map/ClientMapProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace adapter {
                /**
                 * Abstracts the Hazelcast data structures with Near Cache support for the Near Cache usage.
                 */
                template <typename K, typename V>
                class IMapDataStructureAdapter : public DataStructureAdapter<K, V> {
                public:
                    IMapDataStructureAdapter(map::ClientMapProxy<K, V> &imap) : map(imap) {
                    }

                    virtual ~IMapDataStructureAdapter() {
                    }

                    void clear() {
                        map.clear();
                    }

                    void set(const K &key, const V &value) {
                        map.set(key, value);
                    }

                    boost::shared_ptr<V> put(const K &key, const V &value) {
                        return map.putIfAbsent(key, value);
                    }

                    boost::shared_ptr<V> get(const K &key) {
                        return map.get(key);
                    }

/*
                    connection::CallFuture getAsync(const K &key) {
                    }
*/

                    void putAll(const std::map<K, V> entries) {
                        map.putAll(entries);
                    }

                    std::map<K, V> getAll(const std::set<K> &keys) {
                        return map.getAll(keys);
                    }

                    void remove(const K &key) {
                        map.remove(key);
                    }

                    bool containsKey(const K &key) const {
                        return map.containsKey(key);
                    }
                private:
                    map::ClientMapProxy<K, V> &map;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_ADAPTER_IMAPDATASTRUCTUREADAPTER_H_ */

