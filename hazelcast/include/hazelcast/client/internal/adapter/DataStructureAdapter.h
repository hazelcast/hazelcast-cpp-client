/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_ADAPTER_DATASTRUCTUREADAPTER_H_
#define HAZELCAST_CLIENT_INTERNAL_ADAPTER_DATASTRUCTUREADAPTER_H_

#include <assert.h>

#include "hazelcast/util/HazelcastDll.h"

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
                 * Note that this is an interface class. See IMapDataStructureAdapter for an implementation.
                 */
                template <typename K, typename V>
                class DataStructureAdapter {
                public:
                    virtual ~DataStructureAdapter() {
                    }

                    virtual void clear() {
                        assert(0);
                    }

                    virtual void set(const K &key, const V &value) {
                        assert(0);
                    }

                    virtual boost::shared_ptr<V> put(const K &key, const V &value) {
                        assert(0);
                        return boost::shared_ptr<V>();
                    }

                    virtual boost::shared_ptr<V> get(const K &key) {
                        assert(0);
                        return boost::shared_ptr<V>();
                    }

/*
                    virtual connection::CallFuture getAsync(const K &key) {
                    }
*/

                    virtual void putAll(const std::map<K, V> entries) {
                        assert(0);
                    }

                    virtual std::map<K, V> getAll(const std::set<K> &keys) {
                        assert(0);
                        return std::map<K, V>();
                    }

                    virtual void remove(const K &key) {
                        assert(0);
                    }

                    /* TODO
                    virtual LocalMapStats getLocalMapStats() = 0;
                    */

                    virtual bool containsKey(const K &key) const {
                        assert(0);
                        return false;
                    }
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_ADAPTER_DATASTRUCTUREADAPTER_H_ */

