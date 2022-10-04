/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/uuid/random_generator.hpp>

#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/protocol/codec/builtin/sql_page_codec.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/sql/impl/query_id.h"
#include "hazelcast/client/sql/sql_column_metadata.h"
#include "hazelcast/client/sql/sql_column_type.h"
#include "hazelcast/client/sql/sql_result.h"
#include "hazelcast/client/sql/sql_statement.h"
#include "hazelcast/client/sql/hazelcast_sql_exception.h"
#include "hazelcast/client/sql/sql_service.h"
#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/sql/impl/query_utils.h"
#include "hazelcast/client/sql/impl/sql_error_code.h"
#include "hazelcast/client/sql/sql_row_metadata.h"
#include "hazelcast/client/sql/sql_page.h"

namespace hazelcast {
namespace client {
namespace sql {

sql_service::sql_service(client::spi::ClientContext& context)
  : client_context_(context)
{
}

sql::impl::query_id
sql_service::create_query_id(
  const std::shared_ptr<connection::Connection>& query_conn)
{
    auto local_id = client_context_.random_uuid();
    auto member_id = query_conn->get_remote_uuid();

    return { member_id, local_id };
}

boost::future<sql_result>
sql_service::execute(const sql_statement& statement)
{
    using protocol::ClientMessage;

    auto query_conn = query_connection();
    sql::impl::query_id qid = create_query_id(query_conn);

    auto request = protocol::codec::sql_execute_encode(
      statement.sql(),
      statement.serialized_parameters_,
      static_cast<int64_t>(statement.timeout().count()),
      static_cast<int32_t>(statement.cursor_buffer_size()),
      statement.schema() ? &statement.schema().value() : nullptr,
      static_cast<byte>(statement.expected_result_type()),
      qid,
      false);

    auto invocation = spi::impl::ClientInvocation::create(
      client_context_, request, "", query_conn);

    auto cursor_buffer_size = statement.cursor_buffer_size();

    return invocation->invoke().then(
      boost::launch::sync,
      [this, query_conn, qid, cursor_buffer_size](
        boost::future<ClientMessage> response_fut) {
          boost::optional<sql_result> result;
          try {
              auto response = response_fut.get();
              return handle_execute_response(
                response, query_conn, qid, cursor_buffer_size);
          } catch (...) {
              rethrow(std::current_exception(), query_conn);
          }
          assert(0);
          return sql_result();
      });
}

boost::future<void>
sql_service::close(const std::shared_ptr<connection::Connection>& connection,
                   impl::query_id id)
{
    auto close_message = protocol::codec::sql_close_encode(id);

    auto invocation = spi::impl::ClientInvocation::create(
      client_context_, close_message, "", connection);
    return invocation->invoke().then(
      [this, connection](boost::future<protocol::ClientMessage> f) {
          try {
              f.get();
          } catch (exception::iexception&) {
              rethrow(std::current_exception(), connection);
          }

          return;
      });
}

sql_service::sql_execute_response_parameters
sql_service::decode_execute_response(protocol::ClientMessage& msg)
{
    static constexpr size_t RESPONSE_UPDATE_COUNT_FIELD_OFFSET =
      protocol::ClientMessage::RESPONSE_HEADER_LEN;
    static constexpr size_t RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET =
      RESPONSE_UPDATE_COUNT_FIELD_OFFSET + protocol::ClientMessage::INT64_SIZE;

    sql_execute_response_parameters response;

    auto initial_frame_header = msg.read_frame_header();
    msg.rd_ptr(protocol::ClientMessage::RESPONSE_HEADER_LEN -
               protocol::ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS);

    response.update_count = msg.get<int64_t>();

    auto frame_len = static_cast<int32_t>(initial_frame_header.frame_len);
    if (frame_len >=
        static_cast<int32_t>(RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET +
                             protocol::ClientMessage::BOOL_SIZE)) {
        response.is_infinite_rows = msg.get<bool>();
        response.is_infinite_rows_exist = true;
        // skip initial_frame
        msg.rd_ptr(frame_len -
                   static_cast<int32_t>(RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET +
                                        protocol::ClientMessage::BOOL_SIZE));
    } else {
        response.is_infinite_rows_exist = false;
        // skip initial_frame
        msg.rd_ptr(static_cast<std::size_t>(frame_len) -
                   RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET);
    }

    response.row_metadata =
      msg.get_nullable<std::vector<sql_column_metadata>>();
    response.first_page = msg.get_nullable<sql::sql_page>(protocol::codec::builtin::sql_page_codec::decode);
    response.error = msg.get_nullable<impl::sql_error>();

    return response;
}

sql_service::sql_fetch_response_parameters
sql_service::decode_fetch_response(protocol::ClientMessage message)
{
    // empty initial frame
    message.skip_frame();

    auto page = message.get_nullable<sql::sql_page>(
      protocol::codec::builtin::sql_page_codec::decode);
    auto error = message.get<boost::optional<impl::sql_error>>();
    return { std::move(page), std::move(error) };
}

sql_result
sql_service::handle_execute_response(
  protocol::ClientMessage& msg,
  std::shared_ptr<connection::Connection> connection,
  impl::query_id id,
  int32_t cursor_buffer_size)
{
    auto response = decode_execute_response(msg);
    if (response.error) {
        BOOST_THROW_EXCEPTION(sql::hazelcast_sql_exception(
          "sql_service::handle_execute_response",
          std::move(response.error->originating_member_id),
          response.error->code,
          std::move(response.error->message),
          std::move(response.error->suggestion)));
    }

    return { &client_context_,
                       this,
                       std::move(connection),
                       id,
                       response.update_count,
                       std::move(response.row_metadata),
                       std::move(response.first_page),
                       cursor_buffer_size };
}

std::shared_ptr<connection::Connection>
sql_service::query_connection()
{
    std::shared_ptr<connection::Connection> connection;
    try {
        auto& cs = client_context_.get_client_cluster_service();
        connection =
          client_context_.get_connection_manager().connection_for_sql(
            [&]() {
                return impl::query_utils::member_of_same_larger_version_group(
                  cs.get_member_list());
            },
            [&](boost::uuids::uuid id) { return cs.get_member(id); });

        if (!connection) {
            exception::query e(
              static_cast<int32_t>(impl::sql_error_code::CONNECTION_PROBLEM),
              "Client is not connected");
            rethrow(std::make_exception_ptr(e));
        }

    } catch (...) {
        rethrow(std::current_exception());
    }

    return connection;
}

void
sql_service::rethrow(const std::exception_ptr& exc_ptr)
{
    // Make sure that access_control is thrown as a top-level exception
    try {
        std::rethrow_if_nested(exc_ptr);
    } catch (exception::access_control&) {
        throw;
    } catch (...) {
        impl::query_utils::throw_public_exception(std::current_exception(),
                                                  client_id());
    }

    impl::query_utils::throw_public_exception(exc_ptr, client_id());
}

void
sql_service::rethrow(std::exception_ptr cause_ptr,
                     const std::shared_ptr<connection::Connection>& connection)
{
    if (!connection->is_alive()) {
        auto msg = (boost::format("Cluster topology changed while a query was "
                                  "executed: Member cannot be reached: %1%") %
                    connection->get_remote_address())
                     .str();
        return impl::query_utils::throw_public_exception(
          std::make_exception_ptr(exception::query(
            static_cast<int32_t>(impl::sql_error_code::CONNECTION_PROBLEM),
            msg,
            std::move(cause_ptr))),
          client_id());
    }

    return rethrow(cause_ptr);
}

boost::uuids::uuid
sql_service::client_id()
{
    return client_context_.get_connection_manager().get_client_uuid();
}

boost::future<sql_page>
sql_service::fetch_page(
  const impl::query_id& q_id,
  int32_t cursor_buffer_size,
  const std::shared_ptr<connection::Connection>& connection)
{
    auto request_message =
      protocol::codec::sql_fetch_encode(q_id, cursor_buffer_size);

    auto response_future = spi::impl::ClientInvocation::create(
                             client_context_, request_message, "", connection)
                             ->invoke();

    return response_future.then(
      boost::launch::sync, [this](boost::future<protocol::ClientMessage> f) {
          boost::optional<sql_page> page;
          try {
              auto response_message = f.get();

              sql_fetch_response_parameters response_params =
                this->decode_fetch_response(std::move(response_message));

              hazelcast::client::sql::sql_service::handle_fetch_response_error(
                std::move(response_params.error));

              page = std::move(response_params.page);
          } catch (exception::iexception&) {
              impl::query_utils::throw_public_exception(
                std::current_exception(), this->client_id());
          }

          return std::move(*std::move(page));
      });
}

void
sql_service::handle_fetch_response_error(boost::optional<impl::sql_error> error)
{
    if (error) {
        throw hazelcast_sql_exception(
          "sql_service::handle_fetch_response_error",
          error->originating_member_id,
          error->code,
          error->message,
          error->suggestion);
    }
}

constexpr std::chrono::milliseconds sql_statement::TIMEOUT_NOT_SET;
constexpr std::chrono::milliseconds sql_statement::TIMEOUT_DISABLED;
constexpr std::chrono::milliseconds sql_statement::DEFAULT_TIMEOUT;
constexpr int32_t sql_statement::DEFAULT_CURSOR_BUFFER_SIZE;

sql_statement::sql_statement(hazelcast_client& client, std::string query)
  : sql_{ std::move(query) }
  , serialized_parameters_{}
  , cursor_buffer_size_{ DEFAULT_CURSOR_BUFFER_SIZE }
  , timeout_{ TIMEOUT_NOT_SET }
  , expected_result_type_{ sql_expected_result_type::any }
  , schema_{}
  , serialization_service_{
      spi::ClientContext(client).get_serialization_service()
  }
{
}

sql_statement::sql_statement(spi::ClientContext& client_context,
                             std::string query)
  : sql_{ std::move(query) }
  , serialized_parameters_{}
  , cursor_buffer_size_{ DEFAULT_CURSOR_BUFFER_SIZE }
  , timeout_{ TIMEOUT_NOT_SET }
  , expected_result_type_{ sql_expected_result_type::any }
  , schema_{}
  , serialization_service_{ client_context.get_serialization_service() }
{
}

const std::string&
sql_statement::sql() const
{
    return sql_;
}

sql_statement&
sql_statement::sql(std::string sql_string)
{
    util::Preconditions::check_not_empty(sql_string, "SQL cannot be empty");

    sql_ = std::move(sql_string);

    return *this;
}

sql_statement&
sql_statement::clear_parameters()
{
    serialized_parameters_.clear();

    return *this;
}

int32_t
sql_statement::cursor_buffer_size() const
{
    return cursor_buffer_size_;
}

sql_statement&
sql_statement::cursor_buffer_size(int32_t size)
{
    util::Preconditions::check_positive(
      size,
      (boost::format("Cursor buffer size must be positive: %s") % size).str());
    cursor_buffer_size_ = size;

    return *this;
}

std::chrono::milliseconds
sql_statement::timeout() const
{
    return timeout_;
}

sql_statement&
sql_statement::timeout(std::chrono::milliseconds timeout)
{
    auto timeout_msecs = timeout.count();
    if (timeout_msecs < 0 && timeout != TIMEOUT_NOT_SET) {
        throw exception::illegal_argument(
          "sql_statement::timeout(std::chrono::milliseconds timeout)",
          (boost::format("Timeout must be non-negative or -1: %1% msecs") %
           timeout_msecs)
            .str());
    }

    timeout_ = timeout;

    return *this;
}

const boost::optional<std::string>&
sql_statement::schema() const
{
    return schema_;
}

sql_statement&
sql_statement::schema(boost::optional<std::string> schema)
{
    schema_ = std::move(schema);
    return *this;
}

sql::sql_expected_result_type
sql_statement::expected_result_type() const
{
    return expected_result_type_;
}

sql_statement&
sql_statement::expected_result_type(sql::sql_expected_result_type type)
{
    expected_result_type_ = type;
    return *this;
}

hazelcast_sql_exception::hazelcast_sql_exception(
  std::string source,
  boost::uuids::uuid originating_member_id,
  int32_t code,
  boost::optional<std::string> message,
  boost::optional<std::string> suggestion,
  std::exception_ptr cause)
  : hazelcast_(std::move(source),
               message ? std::move(message).value() : "",
               "",
               std::move(cause))
  , originating_member_id_(std::move(originating_member_id))
  , code_(code)
  , suggestion_(std::move(suggestion))
{
}

const boost::uuids::uuid&
hazelcast_sql_exception::originating_member_id() const
{
    return originating_member_id_;
}

int32_t
hazelcast_sql_exception::code() const
{
    return code_;
}

const boost::optional<std::string>&
hazelcast_sql_exception::suggestion() const
{
    return suggestion_;
}

namespace impl {

void
query_utils::throw_public_exception(std::exception_ptr exc,
                                    boost::uuids::uuid id)
{
    try {
        std::rethrow_exception(exc);
    } catch (hazelcast_sql_exception& e) {
        throw;
    } catch (exception::query& e) {
        auto originating_member_id = e.originating_member_uuid();
        if (originating_member_id.is_nil()) {
            originating_member_id = id;
        }

        throw hazelcast_sql_exception("query_utils::throw_public_exception",
                                      originating_member_id,
                                      e.code(),
                                      e.get_message(),
                                      e.suggestion(),
                                      exc);
    } catch (exception::iexception& ie) {
        throw hazelcast_sql_exception(
          "query_utils::throw_public_exception",
          id,
          static_cast<int32_t>(sql_error_code::GENERIC),
          ie.get_message(),
          boost::none,
          exc);
    }
}

boost::optional<member>
query_utils::member_of_same_larger_version_group(
  const std::vector<member>& members)
{
    // The members should have at most 2 different version (ignoring the patch
    // version). Find a random member from the larger same-version group.

    // we don't use 2-element array to save on litter
    boost::optional<member::version> version0;
    boost::optional<member::version> version1;
    size_t count0 = 0;
    size_t count1 = 0;

    for (const auto& m : members) {
        if (m.is_lite_member()) {
            continue;
        }
        auto v = m.get_version();
        if (!version0 || *version0 == v) {
            version0 = v;
            ++count0;
        } else if (!version1 || *version1 == v) {
            version1 = v;
            ++count1;
        } else {
            throw exception::runtime(
              "query_utils::member_of_same_larger_version_group",
              (boost::format(
                 "More than 2 distinct member versions found: %1% , %2%") %
               version0 % version1)
                .str());
        }
    }

    assert(count1 == 0 || count0 > 0);

    // no data members
    if (count0 == 0) {
        return boost::none;
    }

    size_t count;
    member::version version;
    if (count0 > count1 || (count0 == count1 && *version0 > *version1)) {
        count = count0;
        version = *version0;
    } else {
        count = count1;
        version = *version1;
    }

    // otherwise return a random member from the larger group
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(0, count - 1);
    auto random_member_index = distribution(generator);
    for (const auto& m : members) {
        if (!m.is_lite_member() && m.get_version() == version) {
            random_member_index--;
            if (random_member_index < 0) {
                return m;
            }
        }
    }

    throw exception::runtime("query_utils::member_of_same_larger_version_group",
                             "should never get here");
}

} // namespace impl

sql_result::sql_result(
  spi::ClientContext* client_context,
  sql_service* service,
  std::shared_ptr<connection::Connection> connection,
  impl::query_id id,
  int64_t update_count,
  boost::optional<std::vector<sql_column_metadata>> columns_metadata,
  boost::optional<sql_page> first_page,
  int32_t cursor_buffer_size)
  : client_context_(client_context)
  , service_(service)
  , connection_(std::move(connection))
  , query_id_(id)
  , update_count_(update_count)
  , first_page_(std::move(first_page))
  , iterator_requested_(false)
  , closed_(false)
  , cursor_buffer_size_(cursor_buffer_size)
{
    if (columns_metadata) {
        row_metadata_.emplace(std::move(*columns_metadata));
        assert(first_page_);
        first_page_->row_metadata(row_metadata_.get_ptr());
        first_page_->serialization_service(
          &client_context_->get_serialization_service());
    }
}

int64_t
sql_result::update_count() const
{
    return update_count_;
}

bool
sql_result::row_set() const
{
    return update_count() == -1;
}

sql_result::page_iterator_type
sql_result::page_iterator()
{
    if (closed_) {
        throw exception::query(static_cast<int32_t>(impl::sql_error_code::CANCELLED_BY_USER),
                               "Query was cancelled by the user");
    }

    if (!first_page_) {
        throw exception::illegal_state(
          "sql_result::page_iterator",
          "This result contains only update count");
    }

    if (iterator_requested_) {
        throw exception::illegal_state("sql_result::page_iterator",
                                       "Iterator can be requested only once");
    }

    iterator_requested_ = true;

    return { this, std::move(first_page_) };
}

boost::future<void>
sql_result::close()
{
    if (closed_) {
        return boost::make_ready_future();
    }

    auto f = service_->close(connection_, query_id_);
    closed_ = true;
    return f;
}

boost::future<sql_page>
sql_result::fetch_page()
{
    return service_->fetch_page(query_id_, cursor_buffer_size_, connection_);
}

const sql_row_metadata&
sql_result::row_metadata() const
{
    if (!row_metadata_) {
        throw exception::illegal_state(
          "sql_result::row_metadata", "This result contains only update count");
    }

    return row_metadata_.value();
}

sql_result::sql_result() = default;

sql_result::page_iterator_type::page_iterator_type(
  sql_result* result,
  boost::optional<sql_page> page)
  : result_(result)
  , page_(std::move(page))
{
    page_->serialization_service(
      &result_->client_context_->get_serialization_service());
    page_->row_metadata(result_->row_metadata_.get_ptr());
}

boost::future<void>
sql_result::page_iterator_type::operator++()
{
    boost::future<sql_page> page_future = result_->fetch_page();

    return page_future.then(
      boost::launch::sync, [this](boost::future<sql_page> page) {
          page_ = std::move(page.get());
          page_->serialization_service(
            &result_->client_context_->get_serialization_service());
          page_->row_metadata(result_->row_metadata_.get_ptr());
      });
}

const boost::optional<sql_page>&
sql_result::page_iterator_type::operator*() const
{
    return page_;
}
sql_result::page_iterator_type::operator bool() const
{
    if (!page_) {
        return false;
    }

    return page_->last_;
}

sql_page::sql_row::sql_row(size_t row_index,
                           const sql_page* page)
  : row_index_(row_index)
  , page_(page)
{
}

std::size_t
sql_page::sql_row::resolve_index(const std::string& column_name) const
{
    auto it = page_->row_metadata_->find_column(column_name);
    if (it == page_->row_metadata_->end()) {
        throw exception::illegal_argument(
          "sql_page::get_object(const std::string &)",
          (boost::format("Column %1% doesn't exist") % column_name).str());
    }
    auto column_index = it->second;
    return column_index;
}

const sql_row_metadata&
sql_page::sql_row::row_metadata() const
{
    return *page_->row_metadata_;
}

sql_page::sql_page(std::vector<sql_column_type> column_types,
                   std::vector<column> columns,
                   bool last)
  : column_types_(std::move(column_types))
  , columns_(std::move(columns))
  , last_(last)
{
    construct_rows();
}

sql_page::sql_page(sql_page&& rhs) noexcept
  : column_types_(std::move(rhs.column_types_))
  , columns_(std::move(rhs.columns_))
  , last_(rhs.last_)
  , row_metadata_(rhs.row_metadata_)
{
    construct_rows();
}

sql_page::sql_page(const sql_page& rhs) noexcept
  : column_types_(rhs.column_types_)
  , columns_(rhs.columns_)
  , last_(rhs.last_)
  , row_metadata_(rhs.row_metadata_)
{
    construct_rows();
}

sql_page&
sql_page::operator=(sql_page&& rhs) noexcept
{
    column_types_ = std::move(rhs.column_types_);
    columns_ = std::move(rhs.columns_);
    last_ = rhs.last_;
    row_metadata_ = rhs.row_metadata_;

    construct_rows();
    return *this;
}

sql_page&
sql_page::operator=(const sql_page& rhs) noexcept
{
    column_types_ = rhs.column_types_;
    columns_ = rhs.columns_;
    last_ = rhs.last_;
    row_metadata_ = rhs.row_metadata_;

    construct_rows();
    return *this;
}

void
sql_page::construct_rows()
{
    auto count = row_count();
    rows_.clear();
    for (size_t i = 0; i < count; ++i) {
        rows_.emplace_back(i, this);
    }
}

const std::vector<sql_column_type>&
sql_page::column_types() const
{
    return column_types_;
}

bool
sql_page::last() const
{
    return last_;
}

std::size_t
sql_page::column_count() const
{
    return column_types_.size();
}

std::size_t
sql_page::row_count() const
{
    return columns_[0].size();
}

const std::vector<sql_page::sql_row>&
sql_page::rows() const
{
    return rows_;
}

void
sql_page::row_metadata(const sql_row_metadata* row_meta)
{
    row_metadata_ = row_meta;
}

void
sql_page::serialization_service(serialization::pimpl::SerializationService* ss)
{
    serialization_service_ = ss;
}

sql_row_metadata::sql_row_metadata(std::vector<sql_column_metadata> columns)
  : columns_(std::move(columns))
{
    assert(!columns_.empty());

    for (std::size_t i = 0; i < columns_.size(); ++i) {
        name_to_index_.emplace(columns_[i].name, i);
    }
}

std::size_t
sql_row_metadata::column_count() const
{
    return columns_.size();
}

const sql_column_metadata&
sql_row_metadata::column(std::size_t index) const
{
    if (index >= columns_.size()) {
        throw exception::index_out_of_bounds(
          "sql_row_metadata::column(std::size_t index)",
          (boost::format("Column index is out of bounds: %1%") % index).str());
    }

    return columns_[index];
}

const std::vector<sql_column_metadata>&
sql_row_metadata::columns() const
{
    return columns_;
}

sql_row_metadata::const_iterator
sql_row_metadata::find_column(const std::string& column_name) const
{
    return name_to_index_.find(column_name);
}

sql_row_metadata::const_iterator
sql_row_metadata::end() const
{
    return name_to_index_.end();
}
bool
operator==(const sql_row_metadata& lhs, const sql_row_metadata& rhs)
{
    return lhs.columns_ == rhs.columns_;
}

bool
operator==(const sql_column_metadata& lhs,
           const sql_column_metadata& rhs) {
    return lhs.name == rhs.name && lhs.type == rhs.type &&
           lhs.nullable == rhs.nullable;
}

} // namespace sql
} // namespace client
} // namespace hazelcast
