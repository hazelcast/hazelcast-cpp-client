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
            * Transactional implementation of IMap#contains_key(Object).
            *
            * @see IMap#contains_key(key)
            */
            template<typename K>
            boost::future<bool> contains_key(const K &key) {
                return contains_key_data(to_data(key));
            }

            /**
            * Transactional implementation of IMap#get(Object).
            *
            * @see IMap#get(keu)
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return to_object<V>(get_data(to_data(key)));
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
                return to_object<R>(put_data(to_data(key), to_data(value)));
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
                return set_data(to_data(key), to_data(value));
            }

            /**
            * Transactional implementation of IMap#putIfAbsent(key, value)
            *
            * The object to be put will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#putIfAbsent(key, value)
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> put_if_absent(const K &key, const V &value) {
                return to_object<R>(put_if_absent_data(to_data(key), to_data(value)));
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
                return to_object<R>(replace_data(to_data(key), to_data(value)));
            }

            /**
            * Transactional implementation of IMap#replace(key, value, oldValue).
            *
            * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#replace(key, value, oldValue)
            */
            template<typename K, typename V, typename N>
            boost::future<bool> replace(const K &key, const V &old_value, const N &new_value) {
                return replace_data(to_data(key), to_data(old_value), to_data(new_value));
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
                return to_object<V>(remove_data(to_data(key)));
            }

            /**
            * Transactional implementation of IMap#delete(key).
            *
            * The object to be deleted will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#delete(key)
            */
            template<typename K>
            boost::future<void> delete_entry(const K &key) {
                return delete_entry_data(to_data(key));
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
                return remove_data(to_data(key), to_data(value));
            }

            /**
            * Transactional implementation of IMap#keySet().
            *
            *
            * @see IMap#keySet()
            */
            template<typename K>
            boost::future<std::vector<K>> key_set() {
                return to_object_vector<K>(key_set_data());
            }

            /**
            * Transactional implementation of IMap#keySet(Predicate) .
            *
            *
            * @see IMap#keySet(predicate)
            */
            template<typename K, typename P>
            boost::future<std::vector<K>> key_set(const P &predicate) {
                return to_object_vector<K>(key_set_data(to_data(predicate)));
            }

            /**
            * Transactional implementation of IMap#values().
            *
            *
            * @see IMap#values()
            */
            template<typename V>
            boost::future<std::vector<V>> values() {
                return to_object_vector<V>(values_data());
            }

            /**
            * Transactional implementation of IMap#values(Predicate) .
            *
            * @see IMap#values(Predicate)
            */
            template<typename V, typename P>
            boost::future<std::vector<V>> values(const P &predicate) {
                return to_object_vector<V>(values_data(to_data(predicate)));
            }

        private:
            TransactionalMap(const std::string &name, txn::TransactionProxy &transaction_proxy)
                    : proxy::TransactionalMapImpl(name, transaction_proxy) {}
        };
    }
}

