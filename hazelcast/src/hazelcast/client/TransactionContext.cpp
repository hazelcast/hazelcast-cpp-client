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
// Created by sancar koyunlu on 8/5/13.
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/connection/Connection.h"

namespace hazelcast {
    namespace client {
        TransactionContext::TransactionContext(spi::impl::ClientTransactionManagerServiceImpl &transactionManager,
                                               const TransactionOptions &txnOptions) : options(txnOptions),
                                                                                       txnConnection(
                                                                                               transactionManager.connect()),
                                                                                       transaction(options,
                                                                                                   transactionManager.getClient(),
                                                                                                   txnConnection) {
        }

        std::string TransactionContext::getTxnId() const {
            return transaction.getTxnId();
        }

        void TransactionContext::beginTransaction() {
            transaction.begin();
        }

        void TransactionContext::commitTransaction() {
            transaction.commit();
        }

        void TransactionContext::rollbackTransaction() {
            transaction.rollback();
        }
    }
}
