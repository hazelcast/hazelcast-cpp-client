//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_TransactionalSetProxy
#define HAZELCAST_TransactionalSetProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API TransactionalSetImpl : public TransactionalObject {
            public:
                TransactionalSetImpl(const std::string& name, txn::TransactionProxy *transactionProxy);

                bool add(const serialization::pimpl::Data& e);

                bool remove(const serialization::pimpl::Data& e);

                int size();
            };
        }
    }
}


#endif //HAZELCAST_TransactionalSetProxy
