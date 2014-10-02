//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TransactionalMultiMap
#define HAZELCAST_TransactionalMultiMap

#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"

namespace hazelcast {
    namespace client {

        /**
        *
        * Transactional implementation of MultiMap.
        *
        * @see MultiMap
        * @param <K> key
        * @param <V> value
        */
        template<typename K, typename V>
        class HAZELCAST_API TransactionalMultiMap : public proxy::TransactionalMultiMapImpl {
            friend class TransactionContext;

        public:
            /**
            * Transactional implementation of Multimap#put(key , value).
            *
            * @see Multimap#put(key , value)
            */
            bool put(const K& key, const V& value) {
                return proxy::TransactionalMultiMapImpl::put(toData(key), toData(value));
            };

            /**
            * Transactional implementation of Multimap#get(key).
            *
            * @see Multimap#get(key)
            */
            std::vector<V> get(const K& key) {
                return toObjectCollection<V>(proxy::TransactionalMultiMapImpl::get(toData(key)));
            };

            /**
            * Transactional implementation of Multimap#remove(key , value).
            *
            * @see Multimap#remove(key , value)
            */
            bool remove(const K& key, const V& value) {
                return proxy::TransactionalMultiMapImpl::remove(toData(key), toData(value));
            };

            /**
            * Transactional implementation of Multimap#remove(key).
            *
            * @see Multimap#remove(key)
            */
            std::vector<V> remove(const K& key) {
                return toObjectCollection<V>(proxy::TransactionalMultiMapImpl::remove(toData(key)));
            };


            /**
            * Transactional implementation of Multimap#valueCount(key).
            *
            * @see Multimap#valueCount(key)
            */
            int valueCount(const K& key) {
                return proxy::TransactionalMultiMapImpl::valueCount(toData(key));
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

