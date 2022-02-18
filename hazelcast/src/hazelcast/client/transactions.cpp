/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/client/txn/client_transaction_util.h>
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/transaction_options.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/proxy/TransactionalMapImpl.h"
#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"
#include "hazelcast/client/proxy/TransactionalListImpl.h"
#include "hazelcast/client/proxy/TransactionalQueueImpl.h"
#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/imap.h"
#include "hazelcast/client/multi_map.h"
#include "hazelcast/client/ilist.h"
#include "hazelcast/client/iqueue.h"
#include "hazelcast/client/iset.h"
#include "hazelcast/client/transaction_context.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/protocol/codec/codecs.h"

namespace hazelcast {
namespace client {
namespace txn {
TransactionProxy::TransactionProxy(
  transaction_options& txn_options,
  spi::ClientContext& client_context,
  std::shared_ptr<connection::Connection> connection)
  : options_(txn_options)
  , client_context_(client_context)
  , connection_(connection)
  , thread_id_(util::get_current_thread_id())
  , state_(TxnState::NO_TXN)
{}

TransactionProxy::TransactionProxy(const TransactionProxy& rhs)
  : options_(rhs.options_)
  , client_context_(rhs.client_context_)
  , connection_(rhs.connection_)
  , thread_id_(rhs.thread_id_)
  , txn_id_(rhs.txn_id_)
  , state_(rhs.state_)
  , start_time_(rhs.start_time_)
{
    transaction_exists_.store(rhs.transaction_exists_.load());
}

boost::uuids::uuid
TransactionProxy::get_txn_id() const
{
    return txn_id_;
}

TxnState
TransactionProxy::get_state() const
{
    return state_;
}

std::chrono::milliseconds
TransactionProxy::get_timeout() const
{
    return options_.get_timeout();
}

boost::future<void>
TransactionProxy::begin()
{
    try {
        if (state_ == TxnState::ACTIVE) {
            BOOST_THROW_EXCEPTION(exception::illegal_state(
              "TransactionProxy::begin()", "Transaction is already active"));
        }
        check_thread();
        if (transaction_exists_) {
            BOOST_THROW_EXCEPTION(
              exception::illegal_state("TransactionProxy::begin()",
                                       "Nested transactions are not allowed!"));
        }
        transaction_exists_.store(true);
        start_time_ = std::chrono::steady_clock::now();
        auto request = protocol::codec::transaction_create_encode(
          std::chrono::duration_cast<std::chrono::milliseconds>(get_timeout())
            .count(),
          options_.get_durability(),
          static_cast<int32_t>(options_.get_transaction_type()),
          thread_id_);
        return invoke(request).then(
          boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
              try {
                  auto msg = f.get();
                  // skip header
                  msg.rd_ptr(msg.RESPONSE_HEADER_LEN);
                  this->txn_id_ = msg.get<boost::uuids::uuid>();
                  this->state_ = TxnState::ACTIVE;
              } catch (exception::iexception&) {
                  transaction_exists_.store(false);
                  throw;
              }
          });
    } catch (exception::iexception&) {
        transaction_exists_.store(false);
        throw;
    }
}

boost::future<void>
TransactionProxy::commit()
{
    try {
        if (state_ != TxnState::ACTIVE) {
            BOOST_THROW_EXCEPTION(exception::illegal_state(
              "TransactionProxy::commit()", "Transaction is not active"));
        }
        state_ = TxnState::COMMITTING;
        check_thread();
        check_timeout();

        auto request =
          protocol::codec::transaction_commit_encode(txn_id_, thread_id_);
        return invoke(request).then(
          boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
              try {
                  f.get();
                  state_ = TxnState::COMMITTED;
              } catch (exception::iexception&) {
                  transaction_exists_.store(false);
                  client_transaction_util::transaction_exception_factory()
                    ->rethrow(std::current_exception(),
                              "TransactionProxy::commit() failed");
              }
          });
    } catch (...) {
        state_ = TxnState::COMMIT_FAILED;
        transaction_exists_.store(false);
        client_transaction_util::transaction_exception_factory()->rethrow(
          std::current_exception(), "TransactionProxy::commit() failed");
        return boost::make_ready_future();
    }
}

boost::future<void>
TransactionProxy::rollback()
{
    try {
        if (state_ == TxnState::NO_TXN || state_ == TxnState::ROLLED_BACK) {
            BOOST_THROW_EXCEPTION(exception::illegal_state(
              "TransactionProxy::rollback()", "Transaction is not active"));
        }
        state_ = TxnState::ROLLING_BACK;
        check_thread();
        try {
            auto request =
              protocol::codec::transaction_rollback_encode(txn_id_, thread_id_);
            return invoke(request).then(
              boost::launch::sync,
              [=](boost::future<protocol::ClientMessage> f) {
                  try {
                      state_ = TxnState::ROLLED_BACK;
                      transaction_exists_.store(false);
                      f.get();
                  } catch (exception::iexception& e) {
                      HZ_LOG(
                        client_context_.get_logger(),
                        warning,
                        boost::str(
                          boost::format(
                            "Exception while rolling back the transaction. "
                            "Exception: %1%") %
                          e));
                  }
              });
        } catch (exception::iexception& exception) {
            HZ_LOG(client_context_.get_logger(),
                   warning,
                   boost::str(boost::format(
                                "Exception while rolling back the transaction. "
                                "Exception: %1%") %
                              exception));
        }
        state_ = TxnState::ROLLED_BACK;
        transaction_exists_.store(false);
    } catch (exception::iexception&) {
        transaction_exists_.store(false);
        client_transaction_util::transaction_exception_factory()->rethrow(
          std::current_exception(), "TransactionProxy::rollback() failed");
    }
    return boost::make_ready_future();
}

serialization::pimpl::SerializationService&
TransactionProxy::get_serialization_service()
{
    return client_context_.get_serialization_service();
}

std::shared_ptr<connection::Connection>
TransactionProxy::get_connection()
{
    return connection_;
}

void
TransactionProxy::check_thread()
{
    if (thread_id_ != util::get_current_thread_id()) {
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "TransactionProxy::checkThread()",
          "Transaction cannot span multiple threads!"));
    }
}

void
TransactionProxy::check_timeout()
{
    if (start_time_ + options_.get_timeout() <
        std::chrono::steady_clock::now()) {
        BOOST_THROW_EXCEPTION(exception::transaction(
          "TransactionProxy::checkTimeout()", "Transaction is timed-out!"));
    }
}

TxnState::TxnState(state value)
  : value(value)
{
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

TxnState::operator int() const
{
    return value;
}

void
TxnState::operator=(int i)
{
    value = values[i];
}

boost::future<protocol::ClientMessage>
TransactionProxy::invoke(protocol::ClientMessage& request)
{
    return client_transaction_util::invoke(
      request,
      boost::uuids::to_string(get_txn_id()),
      client_context_,
      connection_);
}

spi::ClientContext&
TransactionProxy::get_client_context() const
{
    return client_context_;
}

const std::shared_ptr<util::exception_util::runtime_exception_factory>
  client_transaction_util::exceptionFactory(
    new class transaction_exception_factory());

boost::future<protocol::ClientMessage>
client_transaction_util::invoke(
  protocol::ClientMessage& request,
  const std::string& object_name,
  spi::ClientContext& client,
  const std::shared_ptr<connection::Connection>& connection)
{
    try {
        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation =
          spi::impl::ClientInvocation::create(
            client, request, object_name, connection);
        return clientInvocation->invoke();
    } catch (exception::iexception&) {
        transaction_exception_factory()->rethrow(
          std::current_exception(), "ClientTransactionUtil::invoke failed");
        return boost::make_ready_future(protocol::ClientMessage(0));
    }
}

const std::shared_ptr<util::exception_util::runtime_exception_factory>&
client_transaction_util::transaction_exception_factory()
{
    return exceptionFactory;
}

void
client_transaction_util::transaction_exception_factory::rethrow(
  std::exception_ptr throwable,
  const std::string& message)
{
    try {
        std::rethrow_exception(throwable);
    } catch (...) {
        std::throw_with_nested(
          boost::enable_current_exception(exception::transaction(
            "transaction_exceptionFactory::create", message)));
    }
}
} // namespace txn

namespace proxy {
TransactionalMapImpl::TransactionalMapImpl(
  const std::string& name,
  txn::TransactionProxy& transaction_proxy)
  : TransactionalObject(imap::SERVICE_NAME, name, transaction_proxy)
{}

boost::future<bool>
TransactionalMapImpl::contains_key_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmap_containskey_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<bool>(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalMapImpl::get_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmap_get_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<int>
TransactionalMapImpl::size()
{
    auto request = protocol::codec::transactionalmap_size_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<int>(request);
}

boost::future<bool>
TransactionalMapImpl::is_empty()
{
    auto request = protocol::codec::transactionalmap_isempty_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<bool>(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalMapImpl::put_data(const serialization::pimpl::data& key,
                               const serialization::pimpl::data& value)
{

    auto request = protocol::codec::transactionalmap_put_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value,
      std::chrono::duration_cast<std::chrono::milliseconds>(get_timeout())
        .count());

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<void>
TransactionalMapImpl::set_data(const serialization::pimpl::data& key,
                               const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmap_set_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return to_void_future(invoke(request));
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalMapImpl::put_if_absent_data(
  const serialization::pimpl::data& key,
  const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmap_putifabsent_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalMapImpl::replace_data(const serialization::pimpl::data& key,
                                   const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmap_replace_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<bool>
TransactionalMapImpl::replace_data(const serialization::pimpl::data& key,
                                   const serialization::pimpl::data& old_value,
                                   const serialization::pimpl::data& new_value)
{
    auto request = protocol::codec::transactionalmap_replaceifsame_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      old_value,
      new_value);

    return invoke_and_get_future<bool>(request);
}

boost::future<boost::optional<serialization::pimpl::data>>
TransactionalMapImpl::remove_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmap_remove_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<boost::optional<serialization::pimpl::data>>(
      request);
}

boost::future<void>
TransactionalMapImpl::delete_entry_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmap_delete_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return to_void_future(invoke(request));
}

boost::future<bool>
TransactionalMapImpl::remove_data(const serialization::pimpl::data& key,
                                  const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmap_removeifsame_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return invoke_and_get_future<bool>(request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMapImpl::key_set_data()
{
    auto request = protocol::codec::transactionalmap_keyset_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMapImpl::key_set_data(const serialization::pimpl::data& predicate)
{
    auto request = protocol::codec::transactionalmap_keysetwithpredicate_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      predicate);

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMapImpl::values_data()
{
    auto request = protocol::codec::transactionalmap_values_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMapImpl::values_data(const serialization::pimpl::data& predicate)
{
    auto request = protocol::codec::transactionalmap_valueswithpredicate_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      predicate);

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

TransactionalMultiMapImpl::TransactionalMultiMapImpl(
  const std::string& name,
  txn::TransactionProxy& transaction_proxy)
  : TransactionalObject(multi_map::SERVICE_NAME, name, transaction_proxy)
{}

boost::future<bool>
TransactionalMultiMapImpl::put_data(const serialization::pimpl::data& key,
                                    const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmultimap_put_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return invoke_and_get_future<bool>(request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMultiMapImpl::get_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmultimap_get_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<bool>
TransactionalMultiMapImpl::remove(const serialization::pimpl::data& key,
                                  const serialization::pimpl::data& value)
{
    auto request = protocol::codec::transactionalmultimap_removeentry_encode(
      get_name(),
      get_transaction_id(),
      util::get_current_thread_id(),
      key,
      value);

    return invoke_and_get_future<bool>(request);
}

boost::future<std::vector<serialization::pimpl::data>>
TransactionalMultiMapImpl::remove_data(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmultimap_remove_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
      request);
}

boost::future<int>
TransactionalMultiMapImpl::value_count(const serialization::pimpl::data& key)
{
    auto request = protocol::codec::transactionalmultimap_valuecount_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), key);

    return invoke_and_get_future<int>(request);
}

boost::future<int>
TransactionalMultiMapImpl::size()
{
    auto request = protocol::codec::transactionalmultimap_size_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<int>(request);
}

TransactionalListImpl::TransactionalListImpl(const std::string& object_name,
                                             txn::TransactionProxy& context)
  : TransactionalObject(ilist::SERVICE_NAME, object_name, context)
{}

boost::future<bool>
TransactionalListImpl::add(const serialization::pimpl::data& e)
{
    auto request = protocol::codec::transactionallist_add_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), e);

    return invoke_and_get_future<bool>(request);
}

boost::future<bool>
TransactionalListImpl::remove(const serialization::pimpl::data& e)
{
    auto request = protocol::codec::transactionallist_remove_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), e);

    return invoke_and_get_future<bool>(request);
}

boost::future<int>
TransactionalListImpl::size()
{
    auto request = protocol::codec::transactionallist_size_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<int>(request);
}

TransactionalSetImpl::TransactionalSetImpl(
  const std::string& name,
  txn::TransactionProxy& transaction_proxy)
  : TransactionalObject(iset::SERVICE_NAME, name, transaction_proxy)
{}

boost::future<bool>
TransactionalSetImpl::add_data(const serialization::pimpl::data& e)
{
    auto request = protocol::codec::transactionalset_add_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), e);

    return invoke_and_get_future<bool>(request);
}

boost::future<bool>
TransactionalSetImpl::remove_data(const serialization::pimpl::data& e)
{
    auto request = protocol::codec::transactionalset_remove_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id(), e);

    return invoke_and_get_future<bool>(request);
}

boost::future<int>
TransactionalSetImpl::size()
{
    auto request = protocol::codec::transactionalset_size_encode(
      get_name(), get_transaction_id(), util::get_current_thread_id());

    return invoke_and_get_future<int>(request);
}

TransactionalObject::TransactionalObject(const std::string& service_name,
                                         const std::string& object_name,
                                         txn::TransactionProxy& context)
  : proxy::SerializingProxy(context.get_client_context(), object_name)
  , service_name_(service_name)
  , name_(object_name)
  , context_(context)
{}

TransactionalObject::~TransactionalObject() = default;

const std::string&
TransactionalObject::get_service_name()
{
    return service_name_;
}

const std::string&
TransactionalObject::get_name()
{
    return name_;
}

boost::future<void>
TransactionalObject::destroy()
{
    on_destroy();
    auto request =
      protocol::codec::client_destroyproxy_encode(name_, service_name_);
    return to_void_future(
      invoke_on_connection(request, context_.get_connection()));
}

void
TransactionalObject::on_destroy()
{}

boost::uuids::uuid
TransactionalObject::get_transaction_id() const
{
    return context_.get_txn_id();
}

std::chrono::milliseconds
TransactionalObject::get_timeout() const
{
    return context_.get_timeout();
}
} // namespace proxy

transaction_context::transaction_context(
  spi::impl::ClientTransactionManagerServiceImpl& transaction_manager,
  const transaction_options& txn_options)
  : options_(txn_options)
  , txn_connection_(transaction_manager.connect())
  , transaction_(options_, transaction_manager.get_client(), txn_connection_)
{}

boost::uuids::uuid
transaction_context::get_txn_id() const
{
    return transaction_.get_txn_id();
}

boost::future<void>
transaction_context::begin_transaction()
{
    return transaction_.begin();
}

boost::future<void>
transaction_context::commit_transaction()
{
    return transaction_.commit();
}

boost::future<void>
transaction_context::rollback_transaction()
{
    return transaction_.rollback();
}

transaction_options::transaction_options()
  : timeout_(std::chrono::minutes(2))
  , durability_(1)
  , transaction_type_(transaction_type::TWO_PHASE)
{}

transaction_options::transaction_type
transaction_options::get_transaction_type() const
{
    return transaction_type_;
}

transaction_options&
transaction_options::set_transaction_type(transaction_type type)
{
    transaction_type_ = type;
    return *this;
}

std::chrono::milliseconds
transaction_options::get_timeout() const
{
    return timeout_;
}

transaction_options&
transaction_options::set_timeout(std::chrono::milliseconds duration)
{
    if (duration.count() <= 0) {
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "TransactionOptions::setTimeout", "Timeout must be positive!"));
    }
    timeout_ = duration;
    return *this;
}

int
transaction_options::get_durability() const
{
    return durability_;
}

transaction_options&
transaction_options::set_durability(int num_machines)
{
    if (num_machines < 0) {
        BOOST_THROW_EXCEPTION(
          exception::illegal_state("TransactionOptions::setDurability",
                                   "Durability cannot be negative!"));
    }
    this->durability_ = num_machines;
    return *this;
}
} // namespace client
} // namespace hazelcast

namespace std {
std::size_t
hash<std::pair<std::string, std::string>>::operator()(
  const std::pair<std::string, std::string>& val) const noexcept
{
    return std::hash<std::string>{}(val.first + val.second);
}
} // namespace std
