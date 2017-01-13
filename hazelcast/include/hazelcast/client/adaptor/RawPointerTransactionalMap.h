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
// Created by ihsan demir on 24/03/16.
#ifndef HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERRawPointerTransactionalMap_H_
#define HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERRawPointerTransactionalMap_H_

#include "hazelcast/client/TransactionalMap.h"
#include "hazelcast/client/impl/DataArrayImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * Transactional implementation of IMap.
            *
            * @see IMap
            * @param <K> key
            * @param <V> value
            */
            template<typename K, typename V>
            class RawPointerTransactionalMap {
            public:
                RawPointerTransactionalMap(TransactionalMap<K, V> &m) : map(m), serializationService(
                        m.context->getSerializationService()) {
                }

                /**
                * Transactional implementation of IMap#containsKey(Object).
                *
                * @see IMap#containsKey(key)
                */
                bool containsKey(const K &key) {
                    return map.containsKey(key);
                }

                /**
                * Transactional implementation of IMap#get(Object).
                *
                * @see IMap#get(key)
                */
                std::auto_ptr<V> get(const K &key) {
                    return serializationService.toObject<V>(map.getData(serializationService.toData<K>(&key)).get());
                }

                /**
                * Transactional implementation of IMap#size().
                *
                * @see IMap#size()
                */
                int size() {
                    return map.size();
                }

                /**
                * Transactional implementation of IMap#isEmpty().
                *
                * @see IMap#isEmpty()
                */
                bool isEmpty() {
                    return map.isEmpty();
                }

                /**
                * Transactional implementation of IMap#put(Object, Object).
                *
                * The object to be put will be accessible only in the current transaction context till transaction is committed.
                *
                * @see IMap#put(key, value)
                */
                std::auto_ptr<V> put(const K &key, const V &value) {
                    return serializationService.toObject<V>(map.putData(serializationService.toData<K>(&key),
                                                                        serializationService.toData<V>(&value)).get());
                };

                /**
                * Transactional implementation of IMap#set(key, value).
                *
                * The object to be set will be accessible only in the current transaction context till transaction is committed.
                *
                * @see IMap#set(key, value)
                */
                void set(const K &key, const V &value) {
                    map.set(key, value);
                }

                /**
                * Transactional implementation of IMap#putIfAbsent(key, value)
                *
                * The object to be put will be accessible only in the current transaction context till transaction is committed.
                *
                * @see IMap#putIfAbsent(key, value)
                */
                std::auto_ptr<V> putIfAbsent(const K &key, const V &value) {
                    return serializationService.toObject<V>(map.putIfAbsentData(serializationService.toData<K>(&key),
                                                                                serializationService.toData<V>(
                                                                                        &value)).get());
                };

                /**
                * Transactional implementation of IMap#replace(key, value).
                *
                * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
                *
                * @see IMap#replace(key, value)
                */
                std::auto_ptr<V> replace(const K &key, const V &value) {
                    return serializationService.toObject<V>(map.replaceData(serializationService.toData<K>(&key),
                                                                            serializationService.toData<V>(
                                                                                    &value)).get());
                };

                /**
                * Transactional implementation of IMap#replace(key, value, oldValue).
                *
                * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
                *
                * @see IMap#replace(key, value, oldValue)
                */
                bool replace(const K &key, const V &oldValue, const V &newValue) {
                    return map.replace(key, oldValue, newValue);
                };

                /**
                * Transactional implementation of IMap#remove(key).
                *
                * The object to be removed will be removed from only the current transaction context till transaction is committed.
                *
                * @see IMap#remove(key)
                */
                std::auto_ptr<V> remove(const K &key) {
                    return serializationService.toObject<V>(map.removeData(serializationService.toData<K>(&key)).get());
                };

                /**
                * Transactional implementation of IMap#delete(key).
                *
                * The object to be deleted will be removed from only the current transaction context till transaction is committed.
                *
                * @see IMap#delete(keu)
                */

                void deleteEntry(const K &key) {
                    map.deleteEntry(key);
                };

                /**
                * Transactional implementation of IMap#remove(key, value).
                *
                * The object to be removed will be removed from only the current transaction context till transaction is committed.
                *
                * @see IMap#remove(key, value)
                */
                bool remove(const K &key, const V &value) {
                    return map.remove(key, value);
                }

                /**
                * Transactional implementation of IMap#keySet().
                *
                *
                * @see IMap#keySet()
                */
                std::auto_ptr<DataArray<K> > keySet() {
                    return std::auto_ptr<DataArray<K> >(new hazelcast::client::impl::DataArrayImpl<K>(map.keySetData(), serializationService));
                }

                /**
                * Transactional implementation of IMap#keySet(Predicate) .
                *
                *
                * @see IMap#keySet(predicate)
                */
                std::auto_ptr<DataArray<K> > keySet(const serialization::IdentifiedDataSerializable *predicate) {
                    return std::auto_ptr<DataArray<K> >(new hazelcast::client::impl::DataArrayImpl<K>(map.keySetData(predicate), serializationService));
                }

                /**
                * Transactional implementation of IMap#values().
                *
                *
                * @see IMap#values()
                */
                std::auto_ptr<DataArray<V> > values() {
                    return std::auto_ptr<DataArray<V> >(new hazelcast::client::impl::DataArrayImpl<V>(map.valuesData(), serializationService));
                }

                /**
                * Transactional implementation of IMap#values(Predicate) .
                *
                * @see IMap#values(Predicate)
                */
                std::auto_ptr<DataArray<V> > values(const serialization::IdentifiedDataSerializable *predicate) {
                    return std::auto_ptr<DataArray<V> >(new hazelcast::client::impl::DataArrayImpl<V>(map.valuesData(predicate), serializationService));
                }

            private:
                TransactionalMap<K, V> &map;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}


#endif //HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERRawPointerTransactionalMap_H_

