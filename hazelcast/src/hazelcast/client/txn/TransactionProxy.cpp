//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/txn/CreateTxnRequest.h"
#include "hazelcast/client/txn/CommitTxnRequest.h"
#include "hazelcast/client/txn/RollbackTxnRequest.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/Util.h"


namespace hazelcast {
    namespace client {
        namespace txn {
            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext& clientContext, connection::Connection* connection)
            : options(txnOptions)
            , clientContext(clientContext)
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
//                    if (threadFlag.get() != null) {  TODO ask ali abi
//                        throw new IllegalStateException("Nested transactions are not allowed!");
//                    }
//                    threadFlag.set(Boolean.TRUE);
                    startTime = util::getCurrentTimeMillis();

                    CreateTxnRequest request(options);
                    boost::shared_ptr<std::string> response = sendAndReceive<std::string>(request);
                    txnId = *response;
                    state = TxnState::ACTIVE;
                } catch (std::exception &e) {
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
				} catch (exception::IOException &e) {
					state = TxnState::ROLLING_BACK;
                    closeConnection();
                    throw e;
                } catch (exception::ServerException &e) {
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
                    } catch (std::exception &) {
                    }
                    state = TxnState::ROLLED_BACK;
                } catch(std::exception &e) {
                    closeConnection();
                    throw e;
                }
                closeConnection();

            }

            serialization::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }


            spi::ClusterService &TransactionProxy::getClusterService() {
                return clientContext.getClusterService();
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