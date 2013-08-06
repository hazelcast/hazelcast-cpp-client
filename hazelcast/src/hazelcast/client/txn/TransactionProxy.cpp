//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TransactionProxy.h"
#include "CreateTxnRequest.h"
#include "CommitTxnRequest.h"
#include "RollbackTxnRequest.h"
#include "IllegalStateException.h"


namespace hazelcast {
    namespace client {
        namespace txn {
            TransactionProxy::TransactionProxy(TransactionOptions& txnOptions, spi::ClusterService& clusterService,  serialization::SerializationService & serializationService, connection::Connection *connection)
            : options(txnOptions)
            , clusterService(clusterService)
            , serializationService(serializationService)
            , connection(connection)
            , threadId(util::getThreadId())
            , state(TxnState::NO_TXN)
            , startTime(0) {

            };

            std::string TransactionProxy::getTxnId() const {
                return txnId;
            };

            TxnState TransactionProxy::getState() const {
                return state;
            };

            long TransactionProxy::getTimeoutMillis() const {
                return options.getTimeoutMillis();
            };


            void TransactionProxy::begin() {
                try {
                    if (state == TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::begin()", "Transaction is already active");
                    }
                    checkThread();
//                    if (threadFlag.get() != null) {  TODO
//                        throw new IllegalStateException("Nested transactions are not allowed!");
//                    }
//                    threadFlag.set(Boolean.TRUE);
                    startTime = util::getCurrentTimeMillis();

                    CreateTxnRequest request(options);
                    txnId = sendAndReceive<std::string>(request);
                    state = TxnState::ACTIVE;
                } catch (std::exception& e){
                    closeConnection();
                    throw e;
                }

            }

            void TransactionProxy::commit() {
                try {
                    if (state != TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::commit()", "Transaction is not active");
                    }
                    checkThread();
                    checkTimeout();
                    CommitTxnRequest request;
                    sendAndReceive<bool>(request);
                    state = TxnState::COMMITTED;
                } catch (std::exception& e){
                    state = TxnState::ROLLING_BACK;
                    closeConnection();
                    throw e;
                }
                closeConnection();

            }

            void TransactionProxy::rollback() {
                try {
                    if (state == TxnState::NO_TXN || state == TxnState::ROLLED_BACK) {
                        throw exception::IllegalStateException("TransactionProxy::rollback()", "Transaction is not active");
                    }
                    if (state == TxnState::ROLLING_BACK) {
                        state = TxnState::ROLLED_BACK;
                        return;
                    }
                    checkThread();
                    try {
                        RollbackTxnRequest request;
                        sendAndReceive<bool>(request);
                    } catch (std::exception&  ignored) {
                    }
                    state = TxnState::ROLLED_BACK;
                }catch(std::exception& e){
                    closeConnection();
                    throw e;
                }
                closeConnection();

            }

            serialization::SerializationService& TransactionProxy::getSerializationService() {
                return serializationService;
            }


            spi::ClusterService& TransactionProxy::getClusterService() {
                return clusterService;
            }

            connection::Connection *TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::closeConnection() {
//                threadFlag.set(null);
                delete connection;
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getThreadId()) {
                    throw exception::IllegalStateException("TransactionProxy::checkThread()", "Transaction cannot span multiple threads!");
                }
            }

            void TransactionProxy::checkTimeout() {
                if (startTime + options.getTimeoutMillis() < util::getCurrentTimeMillis()) {
                    throw exception::IllegalStateException("TransactionProxy::checkTimeout()", "Transaction is timed-out!");
                }
            }

            TxnState::TxnState(TxnState::states value) {
                values.resize(9);
                values[0] = NO_TXN;
                values[1] = ACTIVE;
                values[2] = PREPARING;
                values[3] = PREPARED;
                values[4] = COMMITTING;
                values[5] = COMMITTED;
                values[6] = COMMIT_FAILED;
                values[7] = ROLLING_BACK;
                values[8] = ROLLED_BACK;
            };

            TxnState::operator int() const {
                return value;
            };

            void TxnState::operator = (int i) {
                value = values[i];
            };


        }
    }
}