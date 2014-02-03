//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalMap
#define HAZELCAST_TransactionalMap

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/map/TxnMapRequest.h"
#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/impl/MapValueCollection.h"
#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class HAZELCAST_API TransactionalMap : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#containsKey(Object)}.
             *
             * @see IMap#containsKey(Object)
            */
            bool containsKey(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::CONTAINS_KEY, data);
                return invoke<bool>(request);
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#get(Object)}.
             *
             * @see IMap#get(Object)
             */
            boost::shared_ptr<V> get(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::GET, data);
                return invoke<V>(request);

            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#size()}.
             *
             * @see com.hazelcast.core.IMap#size()
             */
            int size() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SIZE);
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

            /**
             * Transactional implementation of {@link IMap#isEmpty()}.
             *
             * @see com.hazelcast.core.IMap#isEmpty()
             */
            bool isEmpty() {
                return size() == 0;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#put(Object, Object)}.
             *
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#put(Object, Object)
             */
            boost::shared_ptr<V> put(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT, keyData, valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#set(Object, Object)}.
             *
             * The object to be set will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#set(Object, Object)
             */
            void set(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::SET, keyData, valueData);
                invoke<bool>(request);
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#putIfAbsent(Object, Object)}.
             *
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#putIfAbsent(Object, Object)
             */
            boost::shared_ptr<V> putIfAbsent(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::PUT_IF_ABSENT, keyData, valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#replace(Object, Object)}.
             *
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(Object, Object)
             */
            boost::shared_ptr<V> replace(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, keyData, valueData);
                return invoke<V>(request);

            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#replace(Object, Object, Object)}.
             *
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(Object, Object, Object)
             */
            bool replace(const K &key, const V &oldValue, const V &newValue) {
                serialization::Data keyData = toData(key);
                serialization::Data oldValueData = toData(oldValue);
                serialization::Data newValueData = toData(newValue);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REPLACE, keyData, oldValueData, newValueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;

            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#remove(Object)}.
             *
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(Object)
             */
            boost::shared_ptr<V> remove(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, data);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#delete(Object)}.
             *
             * The object to be deleted will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#delete(Object)
             */

            void deleteEntry(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::DELETE_R, data);
                invoke<bool>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#remove(Object, Object)}.
             *
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(Object, Object)
             */
            bool remove(const K &key, const V &value) {
                serialization::Data data = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::REMOVE, data, valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            /**
            * Transactional implementation of {@link IMap#keySet()}.
            *
            *
            * @see com.hazelcast.core.IMap#keySet()
            */
            std::vector<K> keySet() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET);
                boost::shared_ptr<map::MapKeySet> result = invoke<map::MapKeySet>(request);
                std::vector <serialization::Data> const &keyDataSet = result->getKeySet();
                std::vector<K> keys(keyDataSet.size());
                for (int i = 0; i < keyDataSet.size(); i++) {
                    boost::shared_ptr<K> v = toObject<K>(keyDataSet[i]);
                    keys[i] = *v;
                }
                return keys;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#keySet(com.hazelcast.query.Predicate)} .
             *
             *
             * @see IMap#keySet(com.hazelcast.query.Predicate)
             */
            std::vector<K> keySet(const std::string &predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::KEYSET_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapKeySet> result = invoke<map::MapKeySet>(request);
                std::vector <serialization::Data> const &keyDataSet = result->getKeySet();
                std::vector<K> keys(keyDataSet.size());
                for (int i = 0; i < keyDataSet.size(); i++) {
                    boost::shared_ptr<K> k = toObject<K>(keyDataSet[i]);
                    keys[i] = *k;
                }
                return keys;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#values()}.
             *
             *
             * @see IMap#values()
             */
            std::vector<V> values() {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES);
                boost::shared_ptr<map::MapValueCollection> result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::Data> const &dataValues = result->getValues();
                std::vector<V> values(dataValues.size());
                for (int i = 0; i < dataValues.size(); i++) {
                    boost::shared_ptr<V> v = toObject<V>(dataValues[i]);
                    values[i] = *v;
                }
                return values;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#values(com.hazelcast.query.Predicate)} .
             *
             *
             * @see IMap#values(com.hazelcast.query.Predicate)
             */
            std::vector<V> values(const std::string &predicate) {
                map::TxnMapRequest *request = new map::TxnMapRequest(getName(), map::TxnMapRequestType::VALUES_BY_PREDICATE, predicate);
                boost::shared_ptr<map::MapValueCollection> result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::Data> const &dataValues = result->getValues();
                std::vector<V> values(dataValues.size());
                for (int i = 0; i < dataValues.size(); i++) {
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
