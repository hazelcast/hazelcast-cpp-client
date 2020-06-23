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

#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"

namespace hazelcast {
    namespace client {
        /**
        *
        * Transactional implementation of MultiMap.
        *
        * @see MultiMap
        */
        class HAZELCAST_API TransactionalMultiMap : public proxy::TransactionalMultiMapImpl {
            friend class TransactionContext;

        public:
            /**
            * Transactional implementation of Multimap#put(key , value).
            *
            * @see Multimap#put(key , value)
            */
            template<typename K, typename V>
            boost::future<bool> put(const K &key, const V &value) {
                return proxy::TransactionalMultiMapImpl::putData(toData(key), toData(value));
            }

            /**
            * Transactional implementation of Multimap#get(key).
            *
            * @see Multimap#get(key)
            */
            template<typename K, typename V>
            boost::future<std::vector<V>> get(const K &key) {
                return toObjectVector<V>(proxy::TransactionalMultiMapImpl::getData(toData(key)));
            }

            /**
            * Transactional implementation of Multimap#remove(key , value).
            *
            * @see Multimap#remove(key , value)
            */
            template<typename K, typename V>
            boost::future<bool> remove(const K &key, const V &value) {
                return proxy::TransactionalMultiMapImpl::remove(toData(key), toData(value));
            }

            /**
            * Transactional implementation of Multimap#remove(key).
            *
            * @see Multimap#remove(key)
            */
            template<typename K, typename V>
            boost::future<std::vector<V>> remove(const K &key) {
                return toObjectVector<V>(proxy::TransactionalMultiMapImpl::removeData(toData(key)));
            }

            /**
            * Transactional implementation of Multimap#valueCount(key).
            *
            * @see Multimap#valueCount(key)
            */
            template<typename K>
            boost::future<int> valueCount(const K &key) {
                return proxy::TransactionalMultiMapImpl::valueCount(toData(key));
            }

        private :
            TransactionalMultiMap(const std::string &name, txn::TransactionProxy &transactionProxy)
                    : proxy::TransactionalMultiMapImpl(name, transactionProxy) {}
        };
    }
}

