//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_TransactionalListProxy
#define HAZELCAST_TransactionalListProxy

#include "hazelcast/client/proxy/TransactionalObject.h"


namespace hazelcast {
    namespace client {
        namespace proxy {
            class TransactionalListProxy : public proxy::TransactionalObject {
            public:
                TransactionalListProxy(const std::string& objectName, txn::TransactionProxy *context);

                bool add(const serialization::pimpl::Data& e);

                bool remove(const serialization::pimpl::Data& e);

                int size();

            };
        }
    }
}

#endif //HAZELCAST_TransactionalListProxy
