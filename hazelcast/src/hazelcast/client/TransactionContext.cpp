//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {
        TransactionContext::TransactionContext(spi::ClusterService & clusterService, serialization::SerializationService & serializationService, connection::ConnectionManager & connectionManager, const TransactionOptions & options)
        : CONNECTION_TRY_COUNT(5)
        , connectionManager(connectionManager)
        , options(options)
        , txnConnection(connect())
        , transaction(this->options, clusterService, serializationService, txnConnection) {

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


        connection::Connection *TransactionContext::connect() {
            connection::Connection *conn = NULL;
            for (int i = 0; i < CONNECTION_TRY_COUNT; i++) {
                try {
                    conn = connectionManager.getRandomConnection();
                } catch (exception::IOException&) {
                    continue;
                }
                if (conn != NULL) {
                    break;
                }
            }
            return conn;
        }

    }
}