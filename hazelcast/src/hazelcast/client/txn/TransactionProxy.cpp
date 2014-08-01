//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/txn/CreateTxnRequest.h"
#include "hazelcast/client/txn/CommitTxnRequest.h"
#include "hazelcast/client/txn/RollbackTxnRequest.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/ILogger.h"
#include <ctime> 
namespace hazelcast {
    namespace client {
        namespace txn {
            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext &clientContext, boost::shared_ptr<connection::Connection> connection)
            : options(txnOptions)
            , clientContext(clientContext)
            , connection(connection)
            , threadId(util::getThreadId())
            , state(TxnState::NO_TXN)
            , startTime(0) {

            }

            std::string TransactionProxy::getTxnId() const {
                return txnId;
            }

            TxnState TransactionProxy::getState() const {
                return state;
            }

            int TransactionProxy::getTimeoutSeconds() const {
                return options.getTimeout();
            }


            void TransactionProxy::begin() {
                try {
                    if (state == TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::begin()", "Transaction is already active");
                    }
                    checkThread();
//                    if (threadFlag.get() != null) {  MTODO
//                        throw new IllegalStateException("Nested transactions are not allowed!");
//                    }
//                    threadFlag.set(Boolean.TRUE);
                    startTime = time(NULL);

                    CreateTxnRequest *request = new CreateTxnRequest(options);
                    boost::shared_ptr<std::string> response = invoke<std::string>(request);
                    txnId = *response;
                    state = TxnState::ACTIVE;
                } catch (exception::IException &e) {
                    onTxnEnd();
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
                    CommitTxnRequest *request = new CommitTxnRequest(true);
                    invoke<serialization::pimpl::Void>(request);
                    state = TxnState::COMMITTED;
                } catch (...) {
                    state = TxnState::ROLLING_BACK;
                    onTxnEnd();
                    throw;
                }
                onTxnEnd();

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
                        RollbackTxnRequest *request = new RollbackTxnRequest();
                        invoke<serialization::pimpl::Void>(request);
                    } catch (std::exception &) {
                    }
                    state = TxnState::ROLLED_BACK;
                } catch(exception::IException &e) {
                    onTxnEnd();
                    throw e;
                }
                onTxnEnd();

            }

            serialization::pimpl::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }

            spi::InvocationService &TransactionProxy::getInvocationService() {
                return clientContext.getInvocationService();
            }

            boost::shared_ptr<connection::Connection>TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::onTxnEnd() {
                //threadFlag.set(null);
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getThreadId()) {
                    throw exception::IllegalStateException("TransactionProxy::checkThread()", "Transaction cannot span multiple threads!");
                }
            }

            void TransactionProxy::checkTimeout() {
                time_t current = time(NULL);
				time_t timeoutPoint = startTime + options.getTimeout();
				if (difftime(timeoutPoint, current) < 0) {//timeout - current should be positive 0
					throw exception::IllegalStateException("TransactionProxy::checkTimeout()", "Transaction is timed-out!");
                }
            }

            TxnState::TxnState(State value)
            : value(value) {
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
            }

            TxnState::operator int() const {
                return value;
            }

            void TxnState::operator = (int i) {
                value = values[i];
            }
        }
    }
}
