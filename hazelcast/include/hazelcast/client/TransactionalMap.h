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

#include "hazelcast/client/proxy/TransactionalMapImpl.h"

namespace hazelcast {
    namespace client {
        /**
        * Transactional implementation of IMap.
        *
        * @see IMap
        */
        class HAZELCAST_API TransactionalMap : public proxy::TransactionalMapImpl {
            friend class TransactionContext;
        public:
            /**
            * Transactional implementation of IMap#containsKey(Object).
            *
            * @see IMap#containsKey(key)
            */
            template<typename K>
            boost::future<bool> containsKey(const K &key) {
                return containsKeyData(toData(key));
            }

            /**
            * Transactional implementation of IMap#get(Object).
            *
            * @see IMap#get(keu)
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return toObject<V>(getData(toData(key)));
            }

            /**
            * Transactional implementation of IMap#put(Object, Object).
            *
            * The object to be put will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#put(key, value)
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> put(const K &key, const V &value) {
                return toObject<R>(putData(toData(key), toData(value)));
            }

            /**
            * Transactional implementation of IMap#set(key, value).
            *
            * The object to be set will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#set(key, value)
            */
            template<typename K, typename V>
            boost::future<void> set(const K &key, const V &value) {
                return setData(toData(key), toData(value));
            }

            /**
            * Transactional implementation of IMap#putIfAbsent(key, value)
            *
            * The object to be put will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#putIfAbsent(key, value)
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value) {
                return toObject<V>(putIfAbsentData(toData(key), toData(value)));
            }

            /**
            * Transactional implementation of IMap#replace(key, value).
            *
            * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#replace(key, value)
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> replace(const K &key, const V &value) {
                return toObject<R>(replaceData(toData(key), toData(value)));
            }

            /**
            * Transactional implementation of IMap#replace(key, value, oldValue).
            *
            * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#replace(key, value, oldValue)
            */
            template<typename K, typename V, typename N>
            boost::future<bool> replace(const K &key, const V &oldValue, const N &newValue) {
                return replaceData(toData(key), toData(oldValue), toData(newValue));
            }

            /**
            * Transactional implementation of IMap#remove(key).
            *
            * The object to be removed will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#remove(key)
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> remove(const K &key) {
                return toObject<V>(removeData(toData(key)));
            }

            /**
            * Transactional implementation of IMap#delete(key).
            *
            * The object to be deleted will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#delete(key)
            */
            template<typename K>
            boost::future<void> deleteEntry(const K &key) {
                return deleteEntryData(toData(key));
            }

            /**
            * Transactional implementation of IMap#remove(key, value).
            *
            * The object to be removed will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#remove(key, value)
            */
            template<typename K, typename V>
            boost::future<bool> remove(const K &key, const V &value) {
                return removeData(toData(key), toData(value));
            }

            /**
            * Transactional implementation of IMap#keySet().
            *
            *
            * @see IMap#keySet()
            */
            template<typename K>
            boost::future<std::vector<K>> keySet() {
                return toObjectVector<K>(keySetData());
            }

            /**
            * Transactional implementation of IMap#keySet(Predicate) .
            *
            *
            * @see IMap#keySet(predicate)
            */
            template<typename K, typename P>
            boost::future<std::vector<K>> keySet(const P &predicate) {
                return toObjectVector<K>(keySetData(toData(predicate)));
            }

            /**
            * Transactional implementation of IMap#values().
            *
            *
            * @see IMap#values()
            */
            template<typename V>
            boost::future<std::vector<V>> values() {
                return toObjectVector<V>(valuesData());
            }

            /**
            * Transactional implementation of IMap#values(Predicate) .
            *
            * @see IMap#values(Predicate)
            */
            template<typename V, typename P>
            boost::future<std::vector<V>> values(const P &predicate) {
                return toObjectVector<V>(valuesData(toData(predicate)));
            }

        private:
            TransactionalMap(const std::string &name, txn::TransactionProxy &transactionProxy)
                    : proxy::TransactionalMapImpl(name, transactionProxy) {}
        };
    }
}

