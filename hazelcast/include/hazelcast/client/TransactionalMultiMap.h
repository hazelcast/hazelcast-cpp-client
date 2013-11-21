//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TransactionalMultiMap
#define HAZELCAST_TransactionalMultiMap

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/multimap/TxnMultiMapPutRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapGetRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/multimap/TxnMultiMapRemoveRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapValueCountRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapSizeRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class TransactionalMultiMap : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:

            bool put(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                multimap::TxnMultiMapPutRequest request(getName(), keyData, valueData);
                return invoke<bool>(request);
            };

            std::vector<V> get(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapGetRequest request(getName(), data);
                impl::PortableCollection portableCollection = invoke<impl::PortableCollection>(request);
                vector<serialization::Data> const &dataCollection = portableCollection.getCollection();
                vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    result[i] = toObject<V>(dataCollection[i]);
                }
                return result;
            };

            bool remove(const K &key, const V &value) {
                serialization::Data dataKey = toData(key);
                serialization::Data dataValue = toData(value);
                multimap::TxnMultiMapRemoveRequest request(getName(), dataKey, dataValue);
                return invoke<bool>(request);
            };

            std::vector<V> remove(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapRemoveRequest request(getName(), &data);
                impl::PortableCollection portableCollection = invoke<impl::PortableCollection>(request);
                vector<serialization::Data> const &dataCollection = portableCollection.getCollection();
                vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    result[i] = toData(&(dataCollection[i]));
                }
                return result;
            };


            int valueCount(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapValueCountRequest request(getName(), data);
                return invoke<int>(request);
            }

            int size() {
                multimap::TxnMultiMapSizeRequest request(getName());
                return invoke<int>(request);
            }

            void onDestroy() {
            }

        private :
            TransactionalMultiMap(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

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


#endif //HAZELCAST_TransactionalMultiMap
