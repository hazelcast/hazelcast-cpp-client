//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_TransactionalMultiMap
#define HAZELCAST_TransactionalMultiMap

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/multimap/TxnMultiMapPutRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapGetRequest.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/multimap/TxnMultiMapRemoveRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapValueCountRequest.h"
#include "hazelcast/client/multimap/TxnMultiMapSizeRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"

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
        class HAZELCAST_API TransactionalMultiMap : public proxy::TransactionalObject {
            friend class TransactionContext;
        public:
            /**
             * Transactional implementation of Multimap#put(key , value).
             *
             * @see Multimap#put(key , value)
             */
            bool put(const K &key, const V &value) {
                serialization::pimpl::Data keyData = toData(key);
                serialization::pimpl::Data valueData = toData(value);
                multimap::TxnMultiMapPutRequest *request = new multimap::TxnMultiMapPutRequest(getName(), keyData, valueData);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            };

            /**
             * Transactional implementation of Multimap#get(key).
             *
             * @see Multimap#get(key)
             */
            std::vector<V> get(const K &key) {
                serialization::pimpl::Data data = toData(key);
                multimap::TxnMultiMapGetRequest *request = new multimap::TxnMultiMapGetRequest(getName(), data);
                boost::shared_ptr<impl::PortableCollection> portableCollection = invoke<impl::PortableCollection>(request);
                std::vector<serialization::pimpl::Data> const &dataCollection = portableCollection->getCollection();
                int size = dataCollection.size();
                std::vector<V> result;
                result.resize(size);
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<V> v = toObject<V>(dataCollection[i]);
                    result[i] = *v;
                }
                return result;
            };

            /**
             * Transactional implementation of Multimap#remove(key , value).
             *
             * @see Multimap#remove(key , value)
             */
            bool remove(const K &key, const V &value) {
                serialization::pimpl::Data dataKey = toData(key);
                serialization::pimpl::Data dataValue = toData(value);
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), dataKey, dataValue);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            };

            /**
             * Transactional implementation of Multimap#remove(key).
             *
             * @see Multimap#remove(key)
             */
            std::vector<V> remove(const K &key) {
                serialization::pimpl::Data data = toData(key);
                multimap::TxnMultiMapRemoveRequest *request = new multimap::TxnMultiMapRemoveRequest(getName(), &data);
                boost::shared_ptr<impl::PortableCollection> portableCollection = invoke<impl::PortableCollection>(request);
                std::vector<serialization::pimpl::Data> const &dataCollection = portableCollection->getCollection();
                std::vector<V> result;
                int size = dataCollection.size();
                result.resize(size);
                for (int i = 0; i < size; i++) {
                    result[i] = toData(dataCollection[i]);
                }
                return result;
            };


            /**
             * Transactional implementation of Multimap#valueCount(key).
             *
             * @see Multimap#valueCount(key)
             */
            int valueCount(const K &key) {
                serialization::pimpl::Data data = toData(key);
                multimap::TxnMultiMapValueCountRequest *request = new multimap::TxnMultiMapValueCountRequest(getName(), data);
                boost::shared_ptr<int> cnt = invoke<int>(request);
                return *cnt;
            }

            /**
             * Transactional implementation of Multimap#size().
             *
             * @see Multimap#size()
             */
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

