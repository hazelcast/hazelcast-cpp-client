//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalList
#define HAZELCAST_TransactionalList

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/collection/TxnListAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/collection/TxnListRemoveRequest.h"
#include "hazelcast/client/collection/TxnListSizeRequest.h"
#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        template <typename E>
        class TransactionalList : public proxy::TransactionalObject {
            friend class TransactionContext;

        public:
            bool add(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnListAddRequest request(getName(), &data);
                return invoke<bool>(request);
            }

            bool remove(const E &e) {
                serialization::Data data = toData(e);
                collection::TxnListRemoveRequest request(getName(), &data);
                return invoke<bool>(request);
            }

            int size() {
                collection::TxnListSizeRequest request(getName());
                return invoke<int>(request);
            }

            void onDestroy() {
            }

        private:
            TransactionalList(const std::string &instanceName, txn::TransactionProxy *context)
            :TransactionalObject("hz:impl:listService", instanceName, context) {

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

#endif //HAZELCAST_TransactionalList
