//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalMap
#define HAZELCAST_TransactionalMap

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/map/TxnMapRequest.h"
#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/impl/MapValueCollection.h"
#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {

        /**
         * Transactional implementation of IMap.
         *
         * @see IMap
         * @param <K> key
         * @param <V> value
         */
        template<typename K, typename V>
        class HAZELCAST_API TransactionalMap : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            /**
             * Transactional implementation of IMap#containsKey(Object).
             *
             * @see IMap#containsKey(key)
            */
            bool containsKey(const K &key) {
                serialization::pimpl::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::CONTAINS_KEY, data);
                return invoke<bool>(request);
            }

            /**
             * Transactional implementation of IMap#get(Object).
             *
             * @see IMap#get(keu)
             */
            boost::shared_ptr<V> get(const K &key) {
                serialization::pimpl::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::GET, data);
                return invoke<V>(request);

            }

            /**
             * Transactional implementation of IMap#size().
             *
             * @see IMap#size()
             */
            int size() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SIZE);
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

            /**
             * Transactional implementation of IMap#isEmpty().
             *
             * @see IMap#isEmpty()
             */
            bool isEmpty() {
                return size() == 0;
            }

            /**
             * Transactional implementation of IMap#put(Object, Object).
             *
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#put(key, value)
             */
            boost::shared_ptr<V> put(const K &key, const V &value) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT, keyData, valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of IMap#set(key, value).
             *
             * The object to be set will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#set(key, value)
             */
            void set(const K &key, const V &value) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SET, keyData, valueData);
                invoke<serialization::pimpl::Void>(request);
            }

            /**
             * Transactional implementation of IMap#putIfAbsent(key, value)
             *
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#putIfAbsent(key, value)
             */
            boost::shared_ptr<V> putIfAbsent(const K &key, const V &value) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT_IF_ABSENT, keyData, valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of IMap#replace(key, value).
             *
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(key, value)
             */
            boost::shared_ptr<V> replace(const K &key, const V &value) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, keyData, valueData);
                return invoke<V>(request);

            };

            /**
             * Transactional implementation of IMap#replace(key, value, oldValue).
             *
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(key, value, oldValue)
             */
            bool replace(const K &key, const V &oldValue, const V &newValue) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data oldValueData = toData(oldValue);
                serialization::pimpl::Data newValueData = toData(newValue);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, keyData, oldValueData, newValueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;

            };

            /**
             * Transactional implementation of IMap#remove(key).
             *
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(key)
             */
            boost::shared_ptr<V> remove(const K &key) {
                serialization::pimpl::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, data);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of IMap#delete(key).
             *
             * The object to be deleted will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#delete(keu)
             */

            void deleteEntry(const K &key) {
                serialization::pimpl::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::DELETE_R, data);
                invoke<serialization::pimpl::Void>(request);
            };

            /**
             * Transactional implementation of IMap#remove(key, value).
             *
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(key, value)
             */
            bool remove(const K &key, const V &value) {
                serialization::pimpl::Data data = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, data, valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            /**
            * Transactional implementation of IMap#keySet().
            *
            *
            * @see IMap#keySet()
            */
            std::vector<K> keySet() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET);
                boost::shared_ptr<map::MapKeySet> result = invoke<map::MapKeySet>(request);
                std::vector <serialization::pimpl::Data> const &keyDataSet = result->getKeySet();
                int size = keyDataSet.size();
                std::vector<K> keys(size);
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<K> v = toObject<K>(keyDataSet[i]);
                    keys[i] = *v;
                }
                return keys;
            }

            /**
             * Transactional implementation of IMap#keySet(Predicate) .
             *
             *
             * @see IMap#keySet(predicate)
             */
            std::vector<K> keySet(const std::string &predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapKeySet> result = invoke<map::MapKeySet>(request);
                std::vector <serialization::pimpl::Data> const &keyDataSet = result->getKeySet();
                int size = keyDataSet.size();
                std::vector<K> keys(size);
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<K> k = toObject<K>(keyDataSet[i]);
                    keys[i] = *k;
                }
                return keys;
            }

            /**
             * Transactional implementation of IMap#values().
             *
             *
             * @see IMap#values()
             */
            std::vector<V> values() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES);
                boost::shared_ptr<map::MapValueCollection> result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::pimpl::Data> const &dataValues = result->getValues();
                int size = dataValues.size();
                std::vector<V> values(size);
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<V> v = toObject<V>(dataValues[i]);
                    values[i] = *v;
                }
                return values;
            }

            /**
             * Transactional implementation of IMap#values(Predicate) .
             *
             * @see IMap#values(Predicate)
             */
            std::vector<V> values(const std::string &predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapValueCollection> result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::pimpl::Data> const &dataValues = result->getValues();
                int size = dataValues.size();
                std::vector<V> values(size);
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<V> v = toObject<V>(dataValues[i]);
                    values[i] = *v;
                }
                return values;
            }

        private:
            TransactionalMap(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            void onDestroy() {
            }
        };
    }
}


#endif //HAZELCAST_TransactionalMap
