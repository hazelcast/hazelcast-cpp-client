/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <boost/uuid/uuid_io.hpp>

#include <hazelcast/client/txn/ClientTransactionUtil.h>
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/TransactionalMapImpl.h"
#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"
#include "hazelcast/client/proxy/TransactionalListImpl.h"
#include "hazelcast/client/proxy/TransactionalQueueImpl.h"
#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            TransactionProxy::TransactionProxy(TransactionOptions &txnOptions, spi::ClientContext &clientContext,
                                               std::shared_ptr<connection::Connection> connection)
                    : options(txnOptions), clientContext(clientContext), connection(connection),
                      threadId(util::getCurrentThreadId()), state(TxnState::NO_TXN) {}

            TransactionProxy::TransactionProxy(const TransactionProxy &rhs) : options(rhs.options),
                                                                              clientContext(rhs.clientContext),
                                                                              connection(rhs.connection),
                                                                              threadId(rhs.threadId), txnId(rhs.txnId),
                                                                              state(rhs.state),
                                                                              startTime(rhs.startTime) {
                TRANSACTION_EXISTS.store(rhs.TRANSACTION_EXISTS.load());
            }

            boost::uuids::uuid TransactionProxy::getTxnId() const {
                return txnId;
            }

            TxnState TransactionProxy::getState() const {
                return state;
            }

            std::chrono::steady_clock::duration TransactionProxy::getTimeout() const {
                return options.getTimeout();
            }

            boost::future<void> TransactionProxy::begin() {
                try {
                    if (state == TxnState::ACTIVE) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::begin()",
                                                                               "Transaction is already active"));
                    }
                    checkThread();
                    if (TRANSACTION_EXISTS) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::begin()",
                                                                               "Nested transactions are not allowed!"));
                    }
                    TRANSACTION_EXISTS.store(true);
                    startTime = std::chrono::steady_clock::now();
                    auto request = protocol::codec::transaction_create_encode(
                            std::chrono::duration_cast<std::chrono::milliseconds>(getTimeout()).count(), options.getDurability(),
                            static_cast<int32_t>(options.getTransactionType()), threadId);
                    return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            auto msg = f.get();
                            // skip header
                            msg.rd_ptr(msg.RESPONSE_HEADER_LEN);
                            this->txnId = msg.get<boost::uuids::uuid>();
                            this->state = TxnState::ACTIVE;
                        } catch (exception::IException &) {
                            TRANSACTION_EXISTS.store(false);
                            throw;
                        }
                    });
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    throw;
                }
            }

            boost::future<void> TransactionProxy::commit() {
                try {
                    if (state != TxnState::ACTIVE) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::commit()",
                                                                               "Transaction is not active"));
                    }
                    state = TxnState::COMMITTING;
                    checkThread();
                    checkTimeout();

                    auto request = protocol::codec::transaction_commit_encode(txnId, threadId);
                    return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            f.get();
                            state = TxnState::COMMITTED;
                        } catch (exception::IException &) {
                            TRANSACTION_EXISTS.store(false);
                            ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY()->rethrow(std::current_exception(),
                                                                                            "TransactionProxy::commit() failed");
                        }
                    });
                } catch (...) {
                    state = TxnState::COMMIT_FAILED;
                    TRANSACTION_EXISTS.store(false);
                    ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY()->rethrow(std::current_exception(),
                                                                                    "TransactionProxy::commit() failed");
                    return boost::make_ready_future();
                }
            }

            boost::future<void> TransactionProxy::rollback() {
                try {
                    if (state == TxnState::NO_TXN || state == TxnState::ROLLED_BACK) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::rollback()",
                                                                               "Transaction is not active"));
                    }
                    state = TxnState::ROLLING_BACK;
                    checkThread();
                    try {
                        auto request = protocol::codec::transaction_rollback_encode(txnId, threadId);
                        return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                            try {
                                state = TxnState::ROLLED_BACK;
                                TRANSACTION_EXISTS.store(false);
                                f.get();
                            } catch (exception::IException &e) {
                                clientContext.getLogger().warning("Exception while rolling back the transaction. Exception:",
                                                                  e);
                            }
                        });
                    } catch (exception::IException &exception) {
                        clientContext.getLogger().warning("Exception while rolling back the transaction. Exception:",
                                                          exception);
                    }
                    state = TxnState::ROLLED_BACK;
                    TRANSACTION_EXISTS.store(false);
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS.store(false);
                    ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY()->rethrow(std::current_exception(),
                                                                                    "TransactionProxy::rollback() failed");
                }
                return boost::make_ready_future();
            }

            serialization::pimpl::SerializationService &TransactionProxy::getSerializationService() {
                return clientContext.getSerializationService();
            }

            std::shared_ptr<connection::Connection> TransactionProxy::getConnection() {
                return connection;
            }

            void TransactionProxy::checkThread() {
                if (threadId != util::getCurrentThreadId()) {
                    BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::checkThread()",
                                                                           "Transaction cannot span multiple threads!"));
                }
            }

            void TransactionProxy::checkTimeout() {
                if (startTime + options.getTimeout() < std::chrono::steady_clock::now()) {
                    BOOST_THROW_EXCEPTION(exception::TransactionException("TransactionProxy::checkTimeout()",
                                                                          "Transaction is timed-out!"));
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

            boost::future<protocol::ClientMessage> TransactionProxy::invoke(protocol::ClientMessage &request) {
                return ClientTransactionUtil::invoke(request, boost::uuids::to_string(getTxnId()), clientContext, connection);
            }

            spi::ClientContext &TransactionProxy::getClientContext() const {
                return clientContext;
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> ClientTransactionUtil::exceptionFactory(
                    new TransactionExceptionFactory());

            boost::future<protocol::ClientMessage>
            ClientTransactionUtil::invoke(protocol::ClientMessage &request,
                                          const std::string &objectName,
                                          spi::ClientContext &client,
                                          const std::shared_ptr<connection::Connection> &connection) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, objectName, connection);
                    return clientInvocation->invoke();
                } catch (exception::IException &) {
                    TRANSACTION_EXCEPTION_FACTORY()->rethrow(std::current_exception(),
                                                             "ClientTransactionUtil::invoke failed");
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> &
            ClientTransactionUtil::TRANSACTION_EXCEPTION_FACTORY() {
                return exceptionFactory;
            }

            void
            ClientTransactionUtil::TransactionExceptionFactory::rethrow(std::exception_ptr throwable,
                                                                        const std::string &message) {
                try {
                    std::rethrow_exception(throwable);
                } catch (...) {
                    std::throw_with_nested(
                            boost::enable_current_exception(
                                    exception::TransactionException("TransactionExceptionFactory::create", message)));
                }
            }
        }

        namespace proxy {
            TransactionalMapImpl::TransactionalMapImpl(const std::string &name, txn::TransactionProxy &transactionProxy)
                    : TransactionalObject(IMap::SERVICE_NAME, name, transactionProxy) {}

            boost::future<bool> TransactionalMapImpl::containsKeyData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_containskey_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::getData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_get_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<int> TransactionalMapImpl::size() {
                auto request = protocol::codec::transactionalmap_size_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int>(request);
            }

            boost::future<bool> TransactionalMapImpl::isEmpty() {
                auto request = protocol::codec::transactionalmap_isempty_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> TransactionalMapImpl::putData(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {

                auto request = protocol::codec::transactionalmap_put_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value,
                                std::chrono::duration_cast<std::chrono::milliseconds>(getTimeout()).count());

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<void>
            TransactionalMapImpl::setData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_set_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return toVoidFuture(invoke(request));
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_putifabsent_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::replaceData(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_replace_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<bool> TransactionalMapImpl::replaceData(const serialization::pimpl::Data &key,
                                               const serialization::pimpl::Data &oldValue,
                                               const serialization::pimpl::Data &newValue) {
                auto request = protocol::codec::transactionalmap_replaceifsame_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, oldValue, newValue);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::removeData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_remove_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<void> TransactionalMapImpl::deleteEntryData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_delete_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return toVoidFuture(invoke(request));
            }

            boost::future<bool> TransactionalMapImpl::removeData(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_removeifsame_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMapImpl::keySetData() {
                auto request = protocol::codec::transactionalmap_keyset_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>>
            TransactionalMapImpl::keySetData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::transactionalmap_keysetwithpredicate_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), predicate);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMapImpl::valuesData() {
                auto request = protocol::codec::transactionalmap_values_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>>
            TransactionalMapImpl::valuesData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::transactionalmap_valueswithpredicate_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), predicate);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string &name,
                                                                 txn::TransactionProxy &transactionProxy)
                    : TransactionalObject(MultiMap::SERVICE_NAME, name, transactionProxy) {}

            boost::future<bool> TransactionalMultiMapImpl::putData(const serialization::pimpl::Data &key,
                                                const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmultimap_put_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMultiMapImpl::getData(
                    const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_get_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<bool> TransactionalMultiMapImpl::remove(const serialization::pimpl::Data &key,
                                                   const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmultimap_removeentry_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key, value);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMultiMapImpl::removeData(
                    const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_remove_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<int> TransactionalMultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_valuecount_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), key);

                return invokeAndGetFuture<int>(
                        request);
            }

            boost::future<int> TransactionalMultiMapImpl::size() {
                auto request = protocol::codec::transactionalmultimap_size_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int>(
                        request);
            }

            TransactionalListImpl::TransactionalListImpl(const std::string &objectName, txn::TransactionProxy &context)
                    : TransactionalObject(IList::SERVICE_NAME, objectName, context) {}

            boost::future<bool> TransactionalListImpl::add(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionallist_add_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<bool> TransactionalListImpl::remove(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionallist_remove_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<int> TransactionalListImpl::size() {
                auto request = protocol::codec::transactionallist_size_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int>(
                        request);
            }

            TransactionalSetImpl::TransactionalSetImpl(const std::string &name, txn::TransactionProxy &transactionProxy)
                    : TransactionalObject(ISet::SERVICE_NAME, name, transactionProxy) {}

            boost::future<bool> TransactionalSetImpl::addData(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionalset_add_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetFuture<bool>(request);
            }

            boost::future<bool> TransactionalSetImpl::removeData(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionalset_remove_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetFuture<bool>(
                        request);
            }

            boost::future<int> TransactionalSetImpl::size() {
                auto request = protocol::codec::transactionalset_size_encode(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int>(request);
            }
            
            TransactionalObject::TransactionalObject(const std::string &serviceName, const std::string &objectName,
                                                     txn::TransactionProxy &context)
                    : proxy::SerializingProxy(context.getClientContext(), objectName), serviceName(serviceName),
                      name(objectName), context(context) {}

            TransactionalObject::~TransactionalObject() = default;

            const std::string &TransactionalObject::getServiceName() {
                return serviceName;
            }

            const std::string &TransactionalObject::getName() {
                return name;
            }

            boost::future<void> TransactionalObject::destroy() {
                onDestroy();
                auto request = protocol::codec::client_destroyproxy_encode(name, serviceName);
                return toVoidFuture(invokeOnConnection(request, context.getConnection()));
            }

            void TransactionalObject::onDestroy() {}

            boost::uuids::uuid TransactionalObject::getTransactionId() const {
                return context.getTxnId();
            }

            std::chrono::steady_clock::duration TransactionalObject::getTimeout() const {
                return context.getTimeout();
            }
        }

        TransactionContext::TransactionContext(spi::impl::ClientTransactionManagerServiceImpl &transactionManager,
                                               const TransactionOptions &txnOptions) : options(txnOptions),
                                                                                       txnConnection(
                                                                                               transactionManager.connect()),
                                                                                       transaction(options,
                                                                                                   transactionManager.getClient(),
                                                                                                   txnConnection) {
        }

        boost::uuids::uuid  TransactionContext::getTxnId() const {
            return transaction.getTxnId();
        }

        boost::future<void> TransactionContext::beginTransaction() {
            return transaction.begin();
        }

        boost::future<void> TransactionContext::commitTransaction() {
            return transaction.commit();
        }

        boost::future<void> TransactionContext::rollbackTransaction() {
            return transaction.rollback();
        }

        TransactionOptions::TransactionOptions() : timeout(std::chrono::minutes(2)), durability(1),
                                                   transactionType(TransactionType::TWO_PHASE) {}

        TransactionOptions::TransactionType TransactionOptions::getTransactionType() const {
            return transactionType;
        }

        TransactionOptions &TransactionOptions::setTransactionType(TransactionType type) {
            transactionType = type;
            return *this;
        }

        std::chrono::steady_clock::duration TransactionOptions::getTimeout() const {
            return timeout;
        }

        TransactionOptions &TransactionOptions::setTimeout(std::chrono::steady_clock::duration duration) {
            if (duration.count() <= 0) {
                BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionOptions::setTimeout",
                                                                       "Timeout must be positive!"));
            }
            timeout = duration;
            return *this;
        }

        int TransactionOptions::getDurability() const {
            return durability;
        }

        TransactionOptions &TransactionOptions::setDurability(int numMachines) {
            if (numMachines < 0) {
                BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionOptions::setDurability",
                                                                       "Durability cannot be negative!"));
            }
            this->durability = numMachines;
            return *this;
        }
    }
}

namespace std {
    std::size_t hash<std::pair<std::string, std::string>>::operator()(
            const std::pair<std::string, std::string> &val) const noexcept {
        return std::hash<std::string>{}(val.first + val.second);
    }
}

