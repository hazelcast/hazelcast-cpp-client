/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/6/13.




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
                return proxy::TransactionalSetImpl::add(toData(&e));
            }

            /**
            * Add item from transactional set
            * @param e item
            * @return true if item is remove successfully
            */
            bool remove(const E& e) {
                return proxy::TransactionalSetImpl::remove(toData(&e));
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

