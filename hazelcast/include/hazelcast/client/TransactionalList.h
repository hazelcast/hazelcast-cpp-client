//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TransactionalList
#define HAZELCAST_TransactionalList

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/proxy/TransactionalListProxy.h"

namespace hazelcast {
    namespace client {

        /**
         * Transactional implementation of IList.
         */
        template <typename E>
        class HAZELCAST_API TransactionalList : public proxy::TransactionalListProxy {
            friend class TransactionContext;

        public:
            /**
             * Add new item to transactional list
             * @param e item
             * @return true if item is added successfully
             */
            bool add(const E &e) {
                return proxy::TransactionalListProxy::add(toData(e));
            }

            /**
             * Add item from transactional list
             * @param e item
             * @return true if item is remove successfully
             */
            bool remove(const E &e) {
                return proxy::TransactionalListProxy::remove(toData(e));
            }

            /**
             * Returns the size of the list
             * @return size
             */
            int size() {
                return proxy::TransactionalListProxy::size();
            }

        private:
            TransactionalList(const std::string &instanceName, txn::TransactionProxy *context)
            : proxy::TransactionalListProxy(instanceName, context) {

            }
        };

    }
}

#endif //HAZELCAST_TransactionalList

