//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_TransactionalMultiMapProxy
#define HAZELCAST_TransactionalMultiMapProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API TransactionalMultiMapImpl : public TransactionalObject {
            protected:
                TransactionalMultiMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy);

                bool put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> get(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> remove(const serialization::pimpl::Data& key);

                int valueCount(const serialization::pimpl::Data& key);

                int size();
            };
        }
    }
}


#endif //HAZELCAST_TransactionalMultiMapProxy
