//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_TransactionalMapProxy
#define HAZELCAST_TransactionalMapProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            class HAZELCAST_API TransactionalMapImpl : public TransactionalObject {
            protected:
                TransactionalMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy);

                bool containsKey(const serialization::pimpl::Data& key);

                serialization::pimpl::Data get(const serialization::pimpl::Data& key);

                int size();

                serialization::pimpl::Data put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                serialization::pimpl::Data putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                serialization::pimpl::Data replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                bool replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue);

                serialization::pimpl::Data remove(const serialization::pimpl::Data& key);

                void deleteEntry(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> keySet();

                std::vector<serialization::pimpl::Data> keySet(const std::string& predicate);

                std::vector<serialization::pimpl::Data> values();

                std::vector<serialization::pimpl::Data> values(const std::string& predicate);

            };
        }
    }
}
#endif //HAZELCAST_TransactionalMapProxy
