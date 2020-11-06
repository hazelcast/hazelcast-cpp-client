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
            TransactionProxy::TransactionProxy(TransactionOptions &txn_options, spi::ClientContext &client_context,
                                               std::shared_ptr<connection::Connection> connection)
                    : options_(txn_options), clientContext_(client_context), connection_(connection),
                      threadId_(util::get_current_thread_id()), state_(TxnState::NO_TXN) {}

            TransactionProxy::TransactionProxy(const TransactionProxy &rhs) : options_(rhs.options_),
                                                                              clientContext_(rhs.clientContext_),
                                                                              connection_(rhs.connection_),
                                                                              threadId_(rhs.threadId_), txnId_(rhs.txnId_),
                                                                              state_(rhs.state_),
                                                                              startTime_(rhs.startTime_) {
                TRANSACTION_EXISTS_.store(rhs.TRANSACTION_EXISTS_.load());
            }

            boost::uuids::uuid TransactionProxy::get_txn_id() const {
                return txnId_;
            }

            TxnState TransactionProxy::get_state() const {
                return state_;
            }

            std::chrono::milliseconds TransactionProxy::get_timeout() const {
                return options_.get_timeout();
            }

            boost::future<void> TransactionProxy::begin() {
                try {
                    if (state_ == TxnState::ACTIVE) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::begin()",
                                                                               "Transaction is already active"));
                    }
                    check_thread();
                    if (TRANSACTION_EXISTS_) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::begin()",
                                                                               "Nested transactions are not allowed!"));
                    }
                    TRANSACTION_EXISTS_.store(true);
                    startTime_ = std::chrono::steady_clock::now();
                    auto request = protocol::codec::transaction_create_encode(
                            std::chrono::duration_cast<std::chrono::milliseconds>(get_timeout()).count(), options_.get_durability(),
                            static_cast<int32_t>(options_.get_transaction_type()), threadId_);
                    return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            auto msg = f.get();
                            // skip header
                            msg.rd_ptr(msg.RESPONSE_HEADER_LEN);
                            this->txnId_ = msg.get<boost::uuids::uuid>();
                            this->state_ = TxnState::ACTIVE;
                        } catch (exception::IException &) {
                            TRANSACTION_EXISTS_.store(false);
                            throw;
                        }
                    });
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS_.store(false);
                    throw;
                }
            }

            boost::future<void> TransactionProxy::commit() {
                try {
                    if (state_ != TxnState::ACTIVE) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::commit()",
                                                                               "Transaction is not active"));
                    }
                    state_ = TxnState::COMMITTING;
                    check_thread();
                    check_timeout();

                    auto request = protocol::codec::transaction_commit_encode(txnId_, threadId_);
                    return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                        try {
                            f.get();
                            state_ = TxnState::COMMITTED;
                        } catch (exception::IException &) {
                            TRANSACTION_EXISTS_.store(false);
                            ClientTransactionUtil::transaction_exception_factory()->rethrow(std::current_exception(),
                                                                                            "TransactionProxy::commit() failed");
                        }
                    });
                } catch (...) {
                    state_ = TxnState::COMMIT_FAILED;
                    TRANSACTION_EXISTS_.store(false);
                    ClientTransactionUtil::transaction_exception_factory()->rethrow(std::current_exception(),
                                                                                    "TransactionProxy::commit() failed");
                    return boost::make_ready_future();
                }
            }

            boost::future<void> TransactionProxy::rollback() {
                try {
                    if (state_ == TxnState::NO_TXN || state_ == TxnState::ROLLED_BACK) {
                        BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::rollback()",
                                                                               "Transaction is not active"));
                    }
                    state_ = TxnState::ROLLING_BACK;
                    check_thread();
                    try {
                        auto request = protocol::codec::transaction_rollback_encode(txnId_, threadId_);
                        return invoke(request).then(boost::launch::deferred, [=] (boost::future<protocol::ClientMessage> f) {
                            try {
                                state_ = TxnState::ROLLED_BACK;
                                TRANSACTION_EXISTS_.store(false);
                                f.get();
                            } catch (exception::IException &e) {
                                HZ_LOG(clientContext_.get_logger(), warning,
                                    boost::str(boost::format("Exception while rolling back the transaction. "
                                                             "Exception: %1%")
                                                             % e)
                                );
                            }
                        });
                    } catch (exception::IException &exception) {
                        HZ_LOG(clientContext_.get_logger(), warning,
                            boost::str(boost::format("Exception while rolling back the transaction. "
                                                     "Exception: %1%")
                                                     % exception)
                        );
                    }
                    state_ = TxnState::ROLLED_BACK;
                    TRANSACTION_EXISTS_.store(false);
                } catch (exception::IException &) {
                    TRANSACTION_EXISTS_.store(false);
                    ClientTransactionUtil::transaction_exception_factory()->rethrow(std::current_exception(),
                                                                                    "TransactionProxy::rollback() failed");
                }
                return boost::make_ready_future();
            }

            serialization::pimpl::SerializationService &TransactionProxy::get_serialization_service() {
                return clientContext_.get_serialization_service();
            }

            std::shared_ptr<connection::Connection> TransactionProxy::get_connection() {
                return connection_;
            }

            void TransactionProxy::check_thread() {
                if (threadId_ != util::get_current_thread_id()) {
                    BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionProxy::checkThread()",
                                                                           "Transaction cannot span multiple threads!"));
                }
            }

            void TransactionProxy::check_timeout() {
                if (startTime_ + options_.get_timeout() < std::chrono::steady_clock::now()) {
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
                return ClientTransactionUtil::invoke(request, boost::uuids::to_string(get_txn_id()), clientContext_, connection_);
            }

            spi::ClientContext &TransactionProxy::get_client_context() const {
                return clientContext_;
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> ClientTransactionUtil::exceptionFactory(
                    new TransactionExceptionFactory());

            boost::future<protocol::ClientMessage>
            ClientTransactionUtil::invoke(protocol::ClientMessage &request,
                                          const std::string &object_name,
                                          spi::ClientContext &client,
                                          const std::shared_ptr<connection::Connection> &connection) {
                try {
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, object_name, connection);
                    return clientInvocation->invoke();
                } catch (exception::IException &) {
                    transaction_exception_factory()->rethrow(std::current_exception(),
                                                             "ClientTransactionUtil::invoke failed");
                    return boost::make_ready_future(protocol::ClientMessage(0));
                }
            }

            const std::shared_ptr<util::ExceptionUtil::RuntimeExceptionFactory> &
            ClientTransactionUtil::transaction_exception_factory() {
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
            TransactionalMapImpl::TransactionalMapImpl(const std::string &name, txn::TransactionProxy &transaction_proxy)
                    : TransactionalObject(IMap::SERVICE_NAME, name, transaction_proxy) {}

            boost::future<bool> TransactionalMapImpl::contains_key_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_containskey_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<bool>(request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::get_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_get_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<int> TransactionalMapImpl::size() {
                auto request = protocol::codec::transactionalmap_size_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<int>(request);
            }

            boost::future<bool> TransactionalMapImpl::is_empty() {
                auto request = protocol::codec::transactionalmap_isempty_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>> TransactionalMapImpl::put_data(
                    const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {

                auto request = protocol::codec::transactionalmap_put_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value,
                                std::chrono::duration_cast<std::chrono::milliseconds>(get_timeout()).count());

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<void>
            TransactionalMapImpl::set_data(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_set_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return to_void_future(invoke(request));
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::put_if_absent_data(const serialization::pimpl::Data &key,
                                                  const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_putifabsent_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::replace_data(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_replace_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<bool> TransactionalMapImpl::replace_data(const serialization::pimpl::Data &key,
                                               const serialization::pimpl::Data &old_value,
                                               const serialization::pimpl::Data &new_value) {
                auto request = protocol::codec::transactionalmap_replaceifsame_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, old_value, new_value);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<boost::optional<serialization::pimpl::Data>>
            TransactionalMapImpl::remove_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_remove_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<boost::optional<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<void> TransactionalMapImpl::delete_entry_data(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmap_delete_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return to_void_future(invoke(request));
            }

            boost::future<bool> TransactionalMapImpl::remove_data(const serialization::pimpl::Data &key,
                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmap_removeifsame_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMapImpl::key_set_data() {
                auto request = protocol::codec::transactionalmap_keyset_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>>
            TransactionalMapImpl::key_set_data(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::transactionalmap_keysetwithpredicate_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), predicate);

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMapImpl::values_data() {
                auto request = protocol::codec::transactionalmap_values_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>>
            TransactionalMapImpl::values_data(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::transactionalmap_valueswithpredicate_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), predicate);

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string &name,
                                                                 txn::TransactionProxy &transaction_proxy)
                    : TransactionalObject(MultiMap::SERVICE_NAME, name, transaction_proxy) {}

            boost::future<bool> TransactionalMultiMapImpl::put_data(const serialization::pimpl::Data &key,
                                                const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmultimap_put_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMultiMapImpl::get_data(
                    const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_get_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<bool> TransactionalMultiMapImpl::remove(const serialization::pimpl::Data &key,
                                                   const serialization::pimpl::Data &value) {
                auto request = protocol::codec::transactionalmultimap_removeentry_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key, value);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> TransactionalMultiMapImpl::remove_data(
                    const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_remove_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<std::vector<serialization::pimpl::Data>>(
                        request);
            }

            boost::future<int> TransactionalMultiMapImpl::value_count(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::transactionalmultimap_valuecount_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), key);

                return invoke_and_get_future<int>(
                        request);
            }

            boost::future<int> TransactionalMultiMapImpl::size() {
                auto request = protocol::codec::transactionalmultimap_size_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<int>(
                        request);
            }

            TransactionalListImpl::TransactionalListImpl(const std::string &object_name, txn::TransactionProxy &context)
                    : TransactionalObject(IList::SERVICE_NAME, object_name, context) {}

            boost::future<bool> TransactionalListImpl::add(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionallist_add_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), e);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<bool> TransactionalListImpl::remove(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionallist_remove_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), e);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<int> TransactionalListImpl::size() {
                auto request = protocol::codec::transactionallist_size_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<int>(
                        request);
            }

            TransactionalSetImpl::TransactionalSetImpl(const std::string &name, txn::TransactionProxy &transaction_proxy)
                    : TransactionalObject(ISet::SERVICE_NAME, name, transaction_proxy) {}

            boost::future<bool> TransactionalSetImpl::add_data(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionalset_add_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), e);

                return invoke_and_get_future<bool>(request);
            }

            boost::future<bool> TransactionalSetImpl::remove_data(const serialization::pimpl::Data &e) {
                auto request = protocol::codec::transactionalset_remove_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id(), e);

                return invoke_and_get_future<bool>(
                        request);
            }

            boost::future<int> TransactionalSetImpl::size() {
                auto request = protocol::codec::transactionalset_size_encode(
                                get_name(), get_transaction_id(), util::get_current_thread_id());

                return invoke_and_get_future<int>(request);
            }
            
            TransactionalObject::TransactionalObject(const std::string &service_name, const std::string &object_name,
                                                     txn::TransactionProxy &context)
                    : proxy::SerializingProxy(context.get_client_context(), object_name), serviceName_(service_name),
                      name_(object_name), context_(context) {}

            TransactionalObject::~TransactionalObject() = default;

            const std::string &TransactionalObject::get_service_name() {
                return serviceName_;
            }

            const std::string &TransactionalObject::get_name() {
                return name_;
            }

            boost::future<void> TransactionalObject::destroy() {
                on_destroy();
                auto request = protocol::codec::client_destroyproxy_encode(name_, serviceName_);
                return to_void_future(invoke_on_connection(request, context_.get_connection()));
            }

            void TransactionalObject::on_destroy() {}

            boost::uuids::uuid TransactionalObject::get_transaction_id() const {
                return context_.get_txn_id();
            }

            std::chrono::milliseconds TransactionalObject::get_timeout() const {
                return context_.get_timeout();
            }
        }

        TransactionContext::TransactionContext(spi::impl::ClientTransactionManagerServiceImpl &transaction_manager,
                                               const TransactionOptions &txn_options) : options_(txn_options),
                                                                                       txnConnection_(
                                                                                               transaction_manager.connect()),
                                                                                       transaction_(options_,
                                                                                                   transaction_manager.get_client(),
                                                                                                   txnConnection_) {
        }

        boost::uuids::uuid  TransactionContext::get_txn_id() const {
            return transaction_.get_txn_id();
        }

        boost::future<void> TransactionContext::begin_transaction() {
            return transaction_.begin();
        }

        boost::future<void> TransactionContext::commit_transaction() {
            return transaction_.commit();
        }

        boost::future<void> TransactionContext::rollback_transaction() {
            return transaction_.rollback();
        }

        TransactionOptions::TransactionOptions() : timeout_(std::chrono::minutes(2)), durability_(1),
                                                   transactionType_(TransactionType::TWO_PHASE) {}

        TransactionOptions::TransactionType TransactionOptions::get_transaction_type() const {
            return transactionType_;
        }

        TransactionOptions &TransactionOptions::set_transaction_type(TransactionType type) {
            transactionType_ = type;
            return *this;
        }

        std::chrono::milliseconds TransactionOptions::get_timeout() const {
            return timeout_;
        }

        TransactionOptions &TransactionOptions::set_timeout(std::chrono::milliseconds duration) {
            if (duration.count() <= 0) {
                BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionOptions::setTimeout",
                                                                       "Timeout must be positive!"));
            }
            timeout_ = duration;
            return *this;
        }

        int TransactionOptions::get_durability() const {
            return durability_;
        }

        TransactionOptions &TransactionOptions::set_durability(int num_machines) {
            if (num_machines < 0) {
                BOOST_THROW_EXCEPTION(exception::IllegalStateException("TransactionOptions::setDurability",
                                                                       "Durability cannot be negative!"));
            }
            this->durability_ = num_machines;
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

