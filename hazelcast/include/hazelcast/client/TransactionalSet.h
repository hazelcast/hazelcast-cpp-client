//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalSet
#define HAZELCAST_TransactionalSet


#include "hazelcast/client/serialization/Data.h"
#include "TxnSetAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "TxnSetRemoveRequest.h"
#include "TxnSetSizeRequest.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class TransactionalSet : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            bool add(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnSetAddRequest request(getName(), &data);
                return invoke<bool>(request);
            }

            bool remove(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnSetRemoveRequest request(getName(), &data);
                return invoke<bool>(request);
            }

            int size() {
                collection::TxnSetSizeRequest request(getName());
                return invoke<int>(request);
            }

            void onDestroy() {
            }

        private:
            TransactionalSet(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename Response, typename Request>
            Response invoke(const Request &request) {
                return getContext().template sendAndReceive<Response>(request);
            };

        };

    }
}


#endif //HAZELCAST_TransactionalSet
