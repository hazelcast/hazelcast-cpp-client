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

boost::future<std::shared_ptr<sql_result>>
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
          try {
              auto response = response_fut.get();
              return handle_execute_response(
                response, query_conn, qid, cursor_buffer_size);
          } catch (const std::exception& e) {
              rethrow(e, query_conn);
          }
          assert(0);
          return std::shared_ptr<sql_result>();
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
          } catch (const exception::iexception& e) {
              rethrow(e, connection);
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

    auto column_metadata = msg.get_nullable<std::vector<sql_column_metadata>>();
    if (column_metadata) {
        response.row_metadata = std::make_shared<sql_row_metadata>(std::move(column_metadata.value()));
    }
    auto row_metadata = response.row_metadata;
    auto page = msg.get_nullable<std::shared_ptr<sql::sql_page>>(
      [row_metadata](protocol::ClientMessage& msg) {
          return protocol::codec::builtin::sql_page_codec::decode(msg,
                                                                  row_metadata);
      });
    if (page) {
        response.first_page = *std::move(page);
    }

    response.error = msg.get_nullable<impl::sql_error>();

    return response;
}

sql_service::sql_fetch_response_parameters
sql_service::decode_fetch_response(protocol::ClientMessage message)
{
    // empty initial frame
    message.skip_frame();

    auto page =
      message.get_nullable<std::shared_ptr<sql::sql_page>>([](protocol::ClientMessage& msg) {
          return protocol::codec::builtin::sql_page_codec::decode(msg);
      });
    auto error = message.get<boost::optional<impl::sql_error>>();
    return { std::move(page), std::move(error) };
}

std::shared_ptr<sql_result>
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

    return std::shared_ptr<sql_result>(
      new sql_result(&client_context_,
                     this,
                     std::move(connection),
                     id,
                     response.update_count,
                     std::move(response.row_metadata),
                     std::move(response.first_page),
                     cursor_buffer_size));
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
            try {
                throw exception::query(
                  static_cast<int32_t>(
                    impl::sql_error_code::CONNECTION_PROBLEM),
                  "Client is not connected");
            } catch (const exception::query& e) {
                rethrow(e);
            }
        }

    } catch (const std::exception& e) {
        rethrow(e);
    }

    return connection;
}

void
sql_service::rethrow(const std::exception& e)
{
    // Make sure that access_control is thrown as a top-level exception
    try {
        std::rethrow_if_nested(e);
    } catch (exception::access_control&) {
        throw;
    } catch (...) {
        impl::query_utils::throw_public_exception(std::current_exception(),
                                                  client_id());
    }

    impl::query_utils::throw_public_exception(std::current_exception(),
                                              client_id());
}

void
sql_service::rethrow(const std::exception& cause,
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
            std::make_exception_ptr(cause))),
          client_id());
    }

    return rethrow(cause);
}

boost::uuids::uuid
sql_service::client_id()
{
    return client_context_.get_connection_manager().get_client_uuid();
}

boost::future<std::shared_ptr<sql_page>>
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
          std::shared_ptr<sql_page> page;
          try {
              auto response_message = f.get();

              sql_fetch_response_parameters response_params =
                this->decode_fetch_response(std::move(response_message));

              hazelcast::client::sql::sql_service::handle_fetch_response_error(
                std::move(response_params.error));

              page = std::move(*response_params.page);
          } catch (exception::iexception&) {
              impl::query_utils::throw_public_exception(
                std::current_exception(), this->client_id());
          }

          return page;
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
  : serialized_parameters_{}
  , cursor_buffer_size_{ DEFAULT_CURSOR_BUFFER_SIZE }
  , timeout_{ TIMEOUT_NOT_SET }
  , expected_result_type_{ sql_expected_result_type::any }
  , schema_{}
  , partition_argument_index_{-1}
  , serialization_service_(
      spi::ClientContext(client).get_serialization_service())
{
    sql(std::move(query));
}

sql_statement::sql_statement(spi::ClientContext& client_context,
                             std::string query)
  : serialized_parameters_{}
  , cursor_buffer_size_{ DEFAULT_CURSOR_BUFFER_SIZE }
  , timeout_{ TIMEOUT_NOT_SET }
  , expected_result_type_{ sql_expected_result_type::any }
  , schema_{}
  , partition_argument_index_{-1}
  , serialization_service_(client_context.get_serialization_service())
{
    sql(std::move(query));
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

uint32_t partition_argument_index() const
{    
    return partition_argument_index_;
}

sql_statement&
sql_statement::partition_argument_index(uint32_t partition_argument_index)
{
    if (partition_argument_index < -1) {            
        BOOST_THROW_EXCEPTION(client::exception::illegal_argument(
              "The argument index must be >=0, or -1"));            
    }
    partition_argument_index_ = partition_argument_index;
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

std::ostream& operator<<(std::ostream& os, const query_id& id)
{
    os << "query_id{member_id: " << boost::uuids::to_string(id.member_id)
       << " local_id: " << boost::uuids::to_string(id.local_id) << "}";
    return os;
}

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
  std::shared_ptr<sql_row_metadata> row_metadata,
  std::shared_ptr<sql_page> first_page,
  int32_t cursor_buffer_size,
  int32_t partition_argument_index)
  : client_context_(client_context)
  , service_(service)
  , connection_(std::move(connection))
  , query_id_(id)
  , update_count_(update_count)
  , row_metadata_(std::move(row_metadata))
  , first_page_(std::move(first_page))
  , iterator_requested_(false)
  , closed_(false)
  , cursor_buffer_size_(cursor_buffer_size)
  , partition_argument_index_(partition_argument_index)
{
    if (row_metadata_) {
        assert(first_page_);
        first_page_->row_metadata(row_metadata_);
        first_page_->serialization_service(
          &client_context_->get_serialization_service());
        update_count_ = -1;
    } else {
        closed_ = true;
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

sql_result::page_iterator
sql_result::iterator()
{
    check_closed();

    if (!first_page_) {
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "sql_result::iterator", "This result contains only update count"));
    }

    if (iterator_requested_) {
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "sql_result::page_iterator", "Iterator can be requested only once"));
    }

    iterator_requested_ = true;

    return { shared_from_this(), first_page_ };
}

void
sql_result::check_closed() const
{
    if (closed_) {
        impl::query_utils::throw_public_exception(
          std::make_exception_ptr(exception::query(
            static_cast<int32_t>(impl::sql_error_code::CANCELLED_BY_USER),
            "Query was cancelled by the user")),
          service_->client_id());
    }
}

boost::future<void>
sql_result::close()
{
    if (closed_) {
        return boost::make_ready_future();
    }

    auto release_resources = [this](){
        {
            std::lock_guard<std::mutex> guard{ mtx_ };
            closed_ = true;

            connection_.reset();
        }

        row_metadata_.reset();
        first_page_.reset();
    };

    try
    {
        auto f = service_->close(connection_, query_id_);

        release_resources();

        return f;
    } catch (const std::exception& e) {
        release_resources();

        service_->rethrow(e);
    }

    // This should not be reached.
    return boost::make_ready_future();
}

boost::future<std::shared_ptr<sql_page>>
sql_result::fetch_page()
{
    std::lock_guard<std::mutex> guard{ mtx_ };

    check_closed();
    return service_->fetch_page(query_id_, cursor_buffer_size_, connection_);
}

const sql_row_metadata&
sql_result::row_metadata() const
{
    if (!row_metadata_) {
        throw exception::illegal_state(
          "sql_result::row_metadata", "This result contains only update count");
    }

    return *row_metadata_;
}

sql_result::~sql_result()
{
    try {
        close().get();
    } catch (...) {
        // ignore
        HZ_LOG(client_context_->get_logger(),
               info,
               (boost::format("[sql_result::~sql_result()] Exception while "
                              "closing the query result. Query id: %1%") %
                query_id_)
                 .str());
    }
}

sql_result::page_iterator::page_iterator(std::shared_ptr<sql_result> result,
                                         std::shared_ptr<sql_page> first_page)
  : in_progress_{ std::make_shared<std::atomic<bool>>(false) }
  , last_{ std::make_shared<std::atomic<bool>>(false) }
  , row_metadata_{ result->row_metadata_ }
  , serialization_(&result->client_context_->get_serialization_service())
  , result_{ move(result) }
  , first_page_(move(first_page))
{
}

boost::future<std::shared_ptr<sql_page>>
sql_result::page_iterator::next()
{
    result_->check_closed();

    if (first_page_) {
        auto page = move(first_page_);

        page->serialization_service(serialization_);
        page->row_metadata(row_metadata_);
        *last_ = page->last();

        return boost::make_ready_future<std::shared_ptr<sql_page>>(page);
    }

    if (*in_progress_) {
        BOOST_THROW_EXCEPTION(
          exception::illegal_access("sql_result::page_iterator::next",
                                    "Fetch page operation is already in "
                                    "progress so next must not be called."));
    }

    if (*last_) {
        BOOST_THROW_EXCEPTION(exception::no_such_element(
          "sql_result::page_iterator::next",
          "Last page is already retrieved so there are no more pages."));
    }

    *in_progress_ = true;

    auto page_future = result_->fetch_page();

    std::weak_ptr<std::atomic<bool>> last_w{ last_ };
    std::weak_ptr<std::atomic<bool>> in_progress_w{ in_progress_ };
    std::shared_ptr<sql_row_metadata> row_metadata{ row_metadata_ };
    auto result = result_;
    auto serialization_service = serialization_;

    return page_future.then(
      boost::launch::sync,
      [serialization_service, row_metadata, last_w, in_progress_w, result](
        boost::future<std::shared_ptr<sql_page>> page_f) {
          try {
              auto page = page_f.get();

              result->check_closed();
              page->serialization_service(serialization_service);
              page->row_metadata(move(row_metadata));

              auto last = last_w.lock();

              if (last)
                  *last = page->last();

              auto in_progress = in_progress_w.lock();

              if (in_progress)
                  *in_progress = false;

              return page;
          } catch (...) {
              auto in_progress = in_progress_w.lock();

              if (in_progress)
                  *in_progress = false;

              throw;
          }
      });
}

bool
sql_result::page_iterator::has_next() const
{
    result_->check_closed();
    return !*last_;
}

std::size_t
sql_page::page_data::column_count() const
{
    return column_types_.size();
}

std::size_t
sql_page::page_data::row_count() const
{
    return columns_[0].size();
}

sql_page::sql_row::sql_row(size_t row_index,
                           std::shared_ptr<page_data> shared)
  : row_index_(row_index)
  , page_data_(std::move(shared))
{
}

std::size_t
sql_page::sql_row::resolve_index(const std::string& column_name) const
{
    auto it = page_data_->row_metadata_->find_column(column_name);
    if (it == page_data_->row_metadata_->end()) {
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
    return *page_data_->row_metadata_;
}

void
sql_page::sql_row::check_index(size_t index) const
{
    if (index >= row_metadata().column_count()) {
        throw exception::index_out_of_bounds(
          "sql_page::sql_row::check_index",
          (boost::format("Column index is out of range: %1%") % index).str());
    }
}

sql_page::sql_page(std::vector<sql_column_type> column_types,
                   std::vector<column> columns,
                   bool last,
                   std::shared_ptr<sql_row_metadata> row_metadata)
  : page_data_{ new page_data{ std::move(column_types),
                               std::move(columns),
                               std::move(row_metadata),
                               nullptr } }
  , last_(last)
{
}

void
sql_page::construct_rows()
{
    auto count = row_count();
    rows_.clear();
    for (size_t i = 0; i < count; ++i) {
        rows_.emplace_back(i, page_data_);
    }
}

const std::vector<sql_column_type>&
sql_page::column_types() const
{
    return page_data_->column_types_;
}

bool
sql_page::last() const
{
    return last_;
}

std::size_t
sql_page::column_count() const
{
    return page_data_->column_count();
}

std::size_t
sql_page::row_count() const
{
    return page_data_->row_count();
}

const std::vector<sql_page::sql_row>&
sql_page::rows() const
{
    return rows_;
}

void
sql_page::row_metadata(std::shared_ptr<sql_row_metadata> row_meta)
{
    page_data_->row_metadata_ = std::move(row_meta);
}

void
sql_page::serialization_service(serialization::pimpl::SerializationService* ss)
{
    page_data_->serialization_service_ = ss;
}

sql_row_metadata::sql_row_metadata(std::vector<sql_column_metadata> columns)
  : columns_(std::move(columns))
{
    assert(!columns_.empty());

    name_to_index_.reserve(columns_.size());
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
operator==(const sql_column_metadata& lhs, const sql_column_metadata& rhs)
{
    return lhs.name == rhs.name && lhs.type == rhs.type &&
           lhs.nullable == rhs.nullable;
}

} // namespace sql
} // namespace client
} // namespace hazelcast
