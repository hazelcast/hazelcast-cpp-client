//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {
        TransactionContext::TransactionContext(spi::ClientContext &clientContext, const TransactionOptions &options)
        : CONNECTION_TRY_COUNT(5)
        , clientContext(clientContext)
        , options(options)
        , txnConnection(connect())
        , transaction(this->options, clientContext, txnConnection) {

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

        boost::shared_ptr<connection::Connection> TransactionContext::connect() {
            return clientContext.getConnectionManager().getRandomConnection(CONNECTION_TRY_COUNT);
        }

    }
}