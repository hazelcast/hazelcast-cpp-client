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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_

#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace store {
                        template <typename K, typename V>
                        class NearCacheObjectRecordStore : public NearCacheRecordStore<K, V> {
                            // TODO
                        public:
                            NearCacheObjectRecordStore(const std::string &name, const boost::shared_ptr<config::NearCacheConfig> &config,
                                                           serialization::pimpl::SerializationService &ss) {
                            }

                            void initialize() {
                            }

                            virtual boost::shared_ptr<V> get(const boost::shared_ptr<K> &key) {
                                return boost::shared_ptr<V>();
                            }

                            virtual void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<V> &value) {

                            }

                            virtual void put(const boost::shared_ptr<K> &key,
                                             const boost::shared_ptr<serialization::pimpl::Data> &value) {

                            }

                            virtual bool remove(const boost::shared_ptr<K> &key) {
                                return false;
                            }

                            virtual void clear() {

                            }

                            virtual void destroy() {

                            }

                            virtual int size() {
                                return -1;
                            }

                            virtual void doExpiration() {

                            }

                            virtual void doEvictionIfRequired() {
                            }

                            virtual void doEviction() {
                            }

                            virtual void storeKeys() {
                            }

                        };

                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_ */

