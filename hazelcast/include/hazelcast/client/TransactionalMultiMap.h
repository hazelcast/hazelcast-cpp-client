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
//
// Created by sancar koyunlu on 8/6/13.

#ifndef HAZELCAST_TransactionalMultiMap
#define HAZELCAST_TransactionalMultiMap

#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template<typename K, typename V>
            class RawPointerTransactionalMultiMap;
        }

        /**
        *
        * Transactional implementation of MultiMap.
        *
        * @see MultiMap
        * @param <K> key
        * @param <V> value
        */
        template<typename K, typename V>
        class TransactionalMultiMap : public proxy::TransactionalMultiMapImpl {
            friend class TransactionContext;
            friend class adaptor::RawPointerTransactionalMultiMap<K, V>;

        public:
            /**
            * Transactional implementation of Multimap#put(key , value).
            *
            * @see Multimap#put(key , value)
            */
            bool put(const K& key, const V& value) {
                return proxy::TransactionalMultiMapImpl::put(toData(&key), toData(&value));
            };

            /**
            * Transactional implementation of Multimap#get(key).
            *
            * @see Multimap#get(key)
            */
            std::vector<V> get(const K& key) {
                return toObjectCollection<V>(proxy::TransactionalMultiMapImpl::getData(toData(&key)));
            };

            /**
            * Transactional implementation of Multimap#remove(key , value).
            *
            * @see Multimap#remove(key , value)
            */
            bool remove(const K& key, const V& value) {
                return proxy::TransactionalMultiMapImpl::remove(toData(&key), toData(&value));
            };

            /**
            * Transactional implementation of Multimap#remove(key).
            *
            * @see Multimap#remove(key)
            */
            std::vector<V> remove(const K& key) {
                return toObjectCollection<V>(proxy::TransactionalMultiMapImpl::removeData(toData(&key)));
            };


            /**
            * Transactional implementation of Multimap#valueCount(key).
            *
            * @see Multimap#valueCount(key)
            */
            int valueCount(const K& key) {
                return proxy::TransactionalMultiMapImpl::valueCount(toData(&key));
            }

            /**
            * Transactional implementation of Multimap#size().
            *
            * @see Multimap#size()
            */
            int size() {
                return proxy::TransactionalMultiMapImpl::size();
            }

        private :
            TransactionalMultiMap(const std::string& name, txn::TransactionProxy *transactionProxy)
            : proxy::TransactionalMultiMapImpl(name, transactionProxy) {

            }
        };
    }
}


#endif //HAZELCAST_TransactionalMultiMap

