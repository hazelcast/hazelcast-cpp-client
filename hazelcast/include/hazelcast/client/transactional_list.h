/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/proxy/TransactionalListImpl.h"

namespace hazelcast {
    namespace client {
        /**
         * Transactional implementation of ilist.
         */
        class transactional_list : public proxy::TransactionalListImpl {
            friend class transaction_context;
        public:
            /**
             * Add new item to transactional list
             * @param e item
             * @return true if item is added successfully
             */
            template<typename E>
            boost::future<bool> add(const E &e) {
                return proxy::TransactionalListImpl::add(to_data(e));
            }

            /**
             * Add item from transactional list
             * @param e item
             * @return true if item is remove successfully
             */
            template<typename E>
            boost::future<bool> remove(const E &e) {
                return proxy::TransactionalListImpl::remove(to_data(e));
            }

        private:
            transactional_list(const std::string &instance_name, txn::TransactionProxy &context)
                    : proxy::TransactionalListImpl(instance_name, context) {}
        };
    }
}

