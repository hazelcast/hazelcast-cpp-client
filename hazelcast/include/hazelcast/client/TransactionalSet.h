//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalSet
#define HAZELCAST_TransactionalSet


#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/collection/TxnSetAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/collection/TxnSetRemoveRequest.h"
#include "hazelcast/client/collection/TxnSetSizeRequest.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class HAZELCAST_API TransactionalSet : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            bool add(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnSetAddRequest *request = new collection::TxnSetAddRequest(getName(), data);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            bool remove(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnSetRemoveRequest *request = new collection::TxnSetRemoveRequest(getName(), data);
                boost::shared_ptr<bool> success = invoke<bool>(request);
                return *success;
            }

            int size() {
                collection::TxnSetSizeRequest *request = new collection::TxnSetSizeRequest(getName());
                boost::shared_ptr<int> s = invoke<int>(request);
                return *s;
            }

            void onDestroy() {
            }

        private:
            TransactionalSet(const std::string &name, txn::TransactionProxy *transactionProxy)
            :TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

        };

    }
}


#endif //HAZELCAST_TransactionalSet
