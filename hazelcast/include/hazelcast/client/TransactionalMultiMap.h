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
        class HAZELCAST_API TransactionalMultiMap : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:

            bool put(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                multimap::TxnMultiMapPutRequest *request = new multimap::TxnMultiMapPutRequest(getName(), keyData, valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            };

            std::vector<V> get(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapGetRequest *request = new multimap::TxnMultiMapGetRequest(getName(), data);
                boost::shared_ptr<impl::PortableCollection> portableCollection = invoke<impl::PortableCollection>(request);
                std::vector<serialization::Data> const &dataCollection = portableCollection->getCollection();
                std::vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    boost::shared_ptr<V> v = toObject<V>(dataCollection[i]);
                    result[i] = *v;
                }
                return result;
            };

            bool remove(const K &key, const V &value) {
                serialization::Data dataKey = toData(key);
                serialization::Data dataValue = toData(value);
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), dataKey, dataValue);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            };

            std::vector<V> remove(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), &data);
                boost::shared_ptr<impl::PortableCollection> portableCollection = invoke<impl::PortableCollection>(request);
                std::vector<serialization::Data> const &dataCollection = portableCollection->getCollection();
                std::vector<serialization::Data>::iterator it;
                std::vector<V> result;
                result.resize(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    result[i] = toData(dataCollection[i]);
                }
                return result;
            };


            int valueCount(const K &key) {
                serialization::Data data = toData(key);
                multimap::TxnMultiMapValueCountRequest *request = new multimap::TxnMultiMapValueCountRequest(getName(), data);
                boost::shared_ptr<int> cnt = invoke<int>(request);
                return *cnt;
            }

            int size() {
                multimap::TxnMultiMapSizeRequest *request = new multimap::TxnMultiMapSizeRequest(getName());
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

        private :
            TransactionalMultiMap(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

            }

            void onDestroy() {
            }

        };
    }
}


#endif //HAZELCAST_TransactionalMultiMap
