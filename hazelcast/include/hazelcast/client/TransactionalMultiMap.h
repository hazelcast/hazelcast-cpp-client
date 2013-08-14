//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TransactionalMultiMap
#define HAZELCAST_TransactionalMultiMap

#include "Data.h"
#include "TxnMultiMapPutRequest.h"
#include "TxnMultiMapGetRequest.h"
#include "PortableCollection.h"
#include "TxnMultiMapRemoveRequest.h"
#include "TxnMultiMapValueCountRequest.h"
#include "TxnMultiMapSizeRequest.h"
#include "TransactionProxy.h"

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class TransactionalMultiMap {
            friend class TransactionContext;

        public:

            bool put(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                collection::TxnMultiMapPutRequest request(name, &keyData, &valueData);
                return invoke<bool>(request);
            };

            std::vector<V> get(const K& key) {
                serialization::Data data = toData(key);
                collection::TxnMultiMapGetRequest request(name, &data);
                impl::PortableCollection portableCollection = invoke<impl::PortableCollection>(request);
                vector<serialization::Data> const & dataCollection = portableCollection.getCollection();
                vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                std::transform(dataCollection.begin(), dataCollection.end(), result.begin(), toData);
                return result;
            };

            bool remove(const K& key, const V& value) {
                serialization::Data dataKey = toData(key);
                serialization::Data dataValue = toData(value);
                collection::TxnMultiMapRemoveRequest request(name, &dataKey, &dataValue);
                return invoke<bool>(request);
            };

            std::vector<V> remove(const K& key) {
                serialization::Data data = toData(key);
                collection::TxnMultiMapRemoveRequest request(name, &data);
                impl::PortableCollection portableCollection = invoke<impl::PortableCollection>(request);
                vector<serialization::Data> const & dataCollection = portableCollection.getCollection();
                vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                std::transform(dataCollection.begin(), dataCollection.end(), result.begin(), toData);
                return result;
            };


            int valueCount(const K& key) {
                serialization::Data data = toData(key);
                collection::TxnMultiMapValueCountRequest request(name, &data);
                return invoke<int>(request);
            }

            int size() {
                collection::TxnMultiMapSizeRequest request(name);
                return invoke<int>(request);
            }

            std::string getName() const {
                return name;
            }

            void onDestroy() {
                //TODO
            }

        private :

            txn::TransactionProxy *transaction;
            std::string name;

            void init(const std::string& name, txn::TransactionProxy *transactionProxy) {
                this->transaction = transactionProxy;
                this->name = name;
            };

            template<typename T>
            serialization::Data toData(const T& object) {
                return transaction->getSerializationService().toData<T>(&object);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return transaction->sendAndReceive<Response>(request);
            };

        };
    }
}


#endif //HAZELCAST_TransactionalMultiMap
