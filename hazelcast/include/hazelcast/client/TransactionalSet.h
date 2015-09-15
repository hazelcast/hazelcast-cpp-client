//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TransactionalSet
#define HAZELCAST_TransactionalSet


#include "hazelcast/client/proxy/TransactionalSetImpl.h"

namespace hazelcast {
    namespace client {
        /**
        * Transactional implementation of ISet.
        */
        template<typename E>
        class TransactionalSet : public proxy::TransactionalSetImpl {
            friend class TransactionContext;

        public:
            /**
            * Add new item to transactional set
            * @param e item
            * @return true if item is added successfully
            */
            bool add(const E& e) {
                return proxy::TransactionalSetImpl::add(toData(e));
            }

            /**
            * Add item from transactional set
            * @param e item
            * @return true if item is remove successfully
            */
            bool remove(const E& e) {
                return proxy::TransactionalSetImpl::remove(toData(e));
            }

            /**
            * Returns the size of the set
            * @return size
            */
            int size() {
                return proxy::TransactionalSetImpl::size();
            }

        private:
            TransactionalSet(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalSetImpl(name, transactionProxy) {

            }
        };

    }
}


#endif //HAZELCAST_TransactionalSet

