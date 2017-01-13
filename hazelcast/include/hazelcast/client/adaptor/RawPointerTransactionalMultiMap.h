/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 24/3/16.

#ifndef HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERTRANSACTIONALMULTIMAP_H_
#define HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERTRANSACTIONALMULTIMAP_H_

#include "hazelcast/client/TransactionalMultiMap.h"
#include "hazelcast/client/impl/DataArrayImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            *
            * Transactional implementation of MultiMap.
            *
            * @see MultiMap
            * @param <K> key
            * @param <V> value
            */
            template<typename K, typename V>
            class RawPointerTransactionalMultiMap {
            public:
                RawPointerTransactionalMultiMap(TransactionalMultiMap<K, V> &m) : map(m), serializationService(
                        m.context->getSerializationService()) {
                }

                /**
                * Transactional implementation of Multimap#put(key , value).
                *
                * @see Multimap#put(key , value)
                */
                bool put(const K &key, const V &value) {
                    return map.put(key, value);
                };

                /**
                * Transactional implementation of Multimap#get(key).
                *
                * @see Multimap#get(key)
                */
                std::auto_ptr<DataArray<K> > get(const K &key) {
                    return std::auto_ptr<DataArray<K> >(new hazelcast::client::impl::DataArrayImpl<K>(map.getData(serializationService.toData<K>(&key)), serializationService));
                };

                /**
                * Transactional implementation of Multimap#remove(key , value).
                *
                * @see Multimap#remove(key , value)
                */
                bool remove(const K &key, const V &value) {
                    return map.remove(key, value);
                };

                /**
                * Transactional implementation of Multimap#remove(key).
                *
                * @see Multimap#remove(key)
                */
                std::auto_ptr<DataArray<V> > remove(const K &key) {
                    return std::auto_ptr<DataArray<V> >(new hazelcast::client::impl::DataArrayImpl<V>(map.removeData(serializationService.toData<K>(&key)), serializationService));
                };


                /**
                * Transactional implementation of Multimap#valueCount(key).
                *
                * @see Multimap#valueCount(key)
                */
                int valueCount(const K &key) {
                    return map.valueCount(key);
                }

                /**
                * Transactional implementation of Multimap#size().
                *
                * @see Multimap#size()
                */
                int size() {
                    return map.size();
                }

            private :
                TransactionalMultiMap<K, V> &map;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERTRANSACTIONALMULTIMAP_H_

