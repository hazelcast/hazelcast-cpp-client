//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalSet
#define HAZELCAST_TransactionalSet


#include "Data.h"
#include "TxnSetAddRequest.h"
#include "TransactionProxy.h"
#include "TxnSetRemoveRequest.h"
#include "TxnSetSizeRequest.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class TransactionalSet {
            friend class TransactionContext;

        public:
            bool add(const E& e) {
                serialization::Data data = toData(e);
                collection::TxnSetAddRequest request(name, &data);
                return invoke<bool>(request);
            }

            bool remove(const E& e) {
                serialization::Data data = toData(e);
                collection::TxnSetRemoveRequest request(name, &data);
                return invoke<bool>(request);
            }

            int size() {
                collection::TxnSetSizeRequest request(name);
                return invoke<int>(request);
            }

            std::string getName() {
                return name;
            }

            void destroy() {
                collection::CollectionDestroyRequest request(name);
                invoke<bool>(request);
            }

        private:
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


#endif //HAZELCAST_TransactionalSet
