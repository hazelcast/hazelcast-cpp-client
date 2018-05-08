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

#include <hazelcast/client/txn/ClientTransactionUtil.h>
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

#include "hazelcast/client/protocol/codec/TransactionCreateCodec.h"
#include "hazelcast/client/protocol/codec/TransactionCommitCodec.h"
#include "hazelcast/client/protocol/codec/TransactionRollbackCodec.h"

namespace hazelcast {
    namespace client {
        namespace txn {
#define MILLISECOND_IN_A_SECOND 1000

            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext &clientContext,
                                               boost::shared_ptr<connection::Connection> connection)
                    : options(txnOptions), clientContext(clientContext), connection(connection),
                      threadId(util::getCurrentThreadId()), state(TxnState::NO_TXN), startTime(0) {
            }


            TransactionProxy::TransactionProxy(const TransactionProxy &rhs) : options(rhs.options),
                                                                              clientContext(rhs.clientContext),
                                                                              connection(rhs.connection),
                                                                              threadId(rhs.threadId), txnId(rhs.txnId),
                                                                              state(rhs.state),
                                                                              startTime(rhs.startTime) {
                TransactionProxy &nonConstRhs = const_cast<TransactionProxy &>(rhs);

                TRANSACTION_EXISTS = static_cast<bool>(nonConstRhs.TRANSACTION_EXISTS);
            }

            const std::string &TransactionProxy::getTxnId() const {
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
                    if (clientContext.getConnectionManager().getOwnerConnection().get() == NULL) {
                        throw exception::TransactionException("TransactionProxy::begin()",
                                                              "Owner connection needs to be present to begin a transaction");
                    }
                    if (state == TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Transaction is already active");
                    }
                    checkThread();
                    if (TRANSACTION_EXISTS) {
                        throw exception::IllegalStateException("TransactionProxy::begin()",
                                                               "Nested transactions are not allowed!");
                    }
                    TRANSACTION_EXISTS = true;
                    startTime = util::currentTimeMillis();
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::TransactionCreateCodec::encodeRequest(
                            options.getTimeout() * MILLISECOND_IN_A_SECOND, options.getDurability(),
                            options.getTransactionType(), threadId);

                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request);

                    protocol::codec::TransactionCreateCodec::ResponseParameters result =
                            protocol::codec::TransactionCreateCodec::ResponseParameters::decode(*response);
                    txnId = result.response;
                    state = TxnState::ACTIVE;
                } catch (exception::IException &e) {
                    TRANSACTION_EXISTS = false;
                    throw;
                }
            }

            void TransactionProxy::commit() {
                try {
                    if (state != TxnState::ACTIVE) {
                        throw exception::IllegalStateException("TransactionProxy::commit()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::COMMITTING;
                    checkThread();
                    checkTimeout();

                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::TransactionCommitCodec::encodeRequest(txnId, threadId);

                    invoke(request);

                    state = TxnState::COMMITTED;
                } catch (exception::IException &) {
                    state = TxnState::COMMIT_FAILED;
                    TRANSACTION_EXISTS = false;
                    throw;
                }
            }

            void TransactionProxy::rollback() {
                try {
                    if (state == TxnState::NO_TXN || state == TxnState::ROLLED_BACK) {
                        throw exception::IllegalStateException("TransactionProxy::rollback()",
                                                               "Transaction is not active");
                    }
                    state = TxnState::ROLLING_BACK;
                    checkThread();
                    try {
                        std::auto_ptr<protocol::ClientMessage> request =
                                protocol::codec::TransactionRollbackCodec::encodeRequest(txnId, threadId);

                        invoke(request);
                    } catch (exception::IException &exception) {
                        util::ILogger::getLogger().warning()
                                << "Exception while rolling back the transaction. Exception:" << exception;
                    }
                    state = TxnState::ROLLED_BACK;
                    TRANSACTION_EXISTS = false;
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS = false;
                    throw;
                }
            }

            serialization::pimpl::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }

            boost::shared_ptr<connection::Connection> TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getCurrentThreadId()) {
                    throw exception::IllegalStateException("TransactionProxy::checkThread()",
                                                           "Transaction cannot span multiple threads!");
                }
            }

            void TransactionProxy::checkTimeout() {
                time_t current = time(NULL);
                time_t timeoutPoint = startTime + options.getTimeout();
                if (difftime(timeoutPoint, current) < 0) {//timeout - current should be positive 0
                    throw exception::IllegalStateException("TransactionProxy::checkTimeout()",
                                                           "Transaction is timed-out!");
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

            void TxnState::operator=(int i) {
                value = values[i];
            }

            boost::shared_ptr<protocol::ClientMessage> TransactionProxy::invoke(
                    std::auto_ptr<protocol::ClientMessage> request) {
                return ClientTransactionUtil::invoke(request, getTxnId(), clientContext, connection);
            }

            spi::ClientContext &TransactionProxy::getClientContext() const {
                return clientContext;
            }
        }
    }
}
