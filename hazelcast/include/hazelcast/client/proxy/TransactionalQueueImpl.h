//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_TransactionalQueueProxy
#define HAZELCAST_TransactionalQueueProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API TransactionalQueueImpl : public TransactionalObject {
            public:
                TransactionalQueueImpl(const std::string& name, txn::TransactionProxy *transactionProxy);

                bool offer(const serialization::pimpl::Data& e, long timeoutInMillis);

                serialization::pimpl::Data poll(long timeoutInMillis);

                int size();
            };
        }
    }
}

#endif //HAZELCAST_TransactionalQueueProxy
