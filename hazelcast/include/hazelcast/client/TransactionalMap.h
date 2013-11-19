//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalMap
#define HAZELCAST_TransactionalMap

#include "SerializationService.h"
#include "TransactionProxy.h"
#include "ClusterService.h"
#include "TxnMapRequest.h"
#include "MapKeySet.h"
#include "MapValueCollection.h"
#include "TransactionalObject.h"

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class TransactionalMap : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#containsKey(Object)}.
             *
             * @see IMap#containsKey(Object)
            */
            bool containsKey(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::CONTAINS_KEY, &data);
                return invoke<bool>(request);
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#get(Object)}.
             *
             * @see IMap#get(Object)
             */
            V get(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::GET, &data);
                return invoke<V>(request);

            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#size()}.
             *
             * @see com.hazelcast.core.IMap#size()
             */
            int size() {
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::SIZE);
                return invoke<int>(request);
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
             * <p/>
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#put(Object, Object)
             */
            V put(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::PUT, &keyData, &valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#set(Object, Object)}.
             * <p/>
             * The object to be set will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#set(Object, Object)
             */
            void set(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::SET, &keyData, &valueData);
                invoke<bool>(request);
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#putIfAbsent(Object, Object)}.
             * <p/>
             * The object to be put will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#putIfAbsent(Object, Object)
             */
            V putIfAbsent(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::PUT_IF_ABSENT, &keyData, &valueData);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#replace(Object, Object)}.
             * <p/>
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(Object, Object)
             */
            V replace(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::REPLACE, &keyData, &valueData);
                return invoke<V>(request);

            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#replace(Object, Object, Object)}.
             * <p/>
             * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
             *
             * @see IMap#replace(Object, Object, Object)
             */
            bool replace(const K &key, const V &oldValue, const V &newValue) {
                serialization::Data keyData = toData(key);
                serialization::Data oldValueData = toData(oldValue);
                serialization::Data newValueData = toData(newValue);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::REPLACE, &keyData, &oldValueData, &newValueData);
                return invoke<bool>(request);

            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#remove(Object)}.
             * <p/>
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(Object)
             */
            V remove(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::REMOVE, &data);
                return invoke<V>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#delete(Object)}.
             * <p/>
             * The object to be deleted will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#delete(Object)
             */

            void deleteEntry(const K &key) {
                serialization::Data data = toData(key);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::DELETE_R, &data);
                invoke<bool>(request);
            };

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#remove(Object, Object)}.
             * <p/>
             * The object to be removed will be removed from only the current transaction context till transaction is committed.
             *
             * @see IMap#remove(Object, Object)
             */
            bool remove(const K &key, const V &value) {
                serialization::Data data = toData(key);
                serialization::Data valueData = toData(value);
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::REMOVE, &data, &valueData);
                return invoke<bool>(request);
            }

            /**
            * Transactional implementation of {@link IMap#keySet()}.
            * <p/>
            *
            * @see com.hazelcast.core.IMap#keySet()
            */
            std::vector<K> keySet() {
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::KEYSET);
                map::MapKeySet result = invoke<map::MapKeySet>(request);
                std::vector <serialization::Data> const &keyDataSet = result.getKeySet();
                std::vector<K> keys(keyDataSet.size());
                for (int i = 0; i < keyDataSet.size(); i++) {
                    keys[i] = toObject<K>(keyDataSet[i]);
                }
                return keys;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#keySet(com.hazelcast.query.Predicate)} .
             * <p/>
             *
             * @see IMap#keySet(com.hazelcast.query.Predicate)
             */
            std::vector<K> keySet(const std::string &predicate) {
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::KEYSET_BY_PREDICATE, predicate);
                map::MapKeySet result = invoke<map::MapKeySet>(request);
                std::vector <serialization::Data> const &keyDataSet = result.getKeySet();
                std::vector<K> keys(keyDataSet.size());
                for (int i = 0; i < keyDataSet.size(); i++) {
                    keys[i] = toObject<K>(keyDataSet[i]);
                }
                return keys;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#values()}.
             * <p/>
             *
             * @see IMap#values()
             */
            std::vector<V> values() {
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::VALUES);
                map::MapValueCollection result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::Data> const &dataValues = result.getValues();
                std::vector<V> values(dataValues.size());
                for (int i = 0; i < dataValues.size(); i++) {
                    values[i] = toObject<V>(dataValues[i]);
                }
                return values;
            }

            /**
             * Transactional implementation of {@link com.hazelcast.core.IMap#values(com.hazelcast.query.Predicate)} .
             * <p/>
             *
             * @see IMap#values(com.hazelcast.query.Predicate)
             */
            std::vector<V> values(const std::string &predicate) {
                map::TxnMapRequest request(getName(), map::TxnMapRequestType::VALUES_BY_PREDICATE, predicate);
                map::MapValueCollection result = invoke<map::MapValueCollection>(request);
                std::vector <serialization::Data> const &dataValues = result.getValues();
                std::vector<V> values(dataValues.size());
                for (int i = 0; i < dataValues.size(); i++) {
                    values[i] = toObject<V>(dataValues[i]);
                }
                return values;
            }

            void onDestroy() {
            }

        private:
            TransactionalMap(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename T>
            T toObject(const serialization::Data &data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(const Request &request) {
                return getContext().template sendAndReceive<Response>(request);
            };

        };
    }
}


#endif //HAZELCAST_TransactionalMap
