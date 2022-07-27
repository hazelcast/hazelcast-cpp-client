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

namespace hazelcast {
namespace client {
namespace sql {

sql_service::sql_service(client::spi::ClientContext& context)
  : client_context_(context)
{
}

boost::future<sql_result>
sql_service::execute(const sql_statement& statement)
{
    using protocol::ClientMessage;

    auto query_conn = query_connection();

    if (!query_conn) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_client_not_active(
          "sql_service::execute",
          "No connection found for SQL"));
    }

    auto local_id = client_context_.random_uuid();
    auto member_id = query_conn->get_remote_uuid();

    sql::impl::query_id qid{
        uuid_high(member_id),
        uuid_low(member_id),
        uuid_high(local_id),
        uuid_low(local_id),
    };

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

    auto result_fut = invocation->invoke().then(
      boost::launch::sync,
      [query_conn](boost::future<ClientMessage> response_fut) {
          ClientMessage response = response_fut.get();

          auto initial_frame_header = response.read_frame_header();
          response.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN -
                          ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS);

          int64_t update_count = response.get<int64_t>();

          response.rd_ptr(static_cast<int32_t>(initial_frame_header.frame_len) -
                          protocol::ClientMessage::RESPONSE_HEADER_LEN -
                          protocol::ClientMessage::INT64_SIZE);

          auto row_metadata =
            response.get_nullable<std::vector<sql_column_metadata>>();
          auto first_page = response.get_nullable<impl::sql_page>();
          auto error = response.get_nullable<impl::sql_error>();

          if (error) {
              BOOST_THROW_EXCEPTION(
                sql::hazelcast_sql_exception(error->originating_member_id,
                                             error->code,
                                             error->message,
                                             error->suggestion));
          }

          return sql_result{ update_count,
                             std::move(row_metadata),
                             std::move(first_page) };
      });

    return result_fut;
}

std::shared_ptr<connection::Connection> sql_service::query_connection() {
    // TODO: Change as Java
    try {
        auto connection = client_context_.get_connection_manager().connection_for_sql();

        if (!connection) {
            exception::query e(static_cast<int32_t>(impl::sql_error_code::CONNECTION_PROBLEM), "Client is not connected");
            rethrow(std::make_exception_ptr(e), e);
        }

        return connection;
    } catch (exception::iexception &e) {
        rethrow(std::current_exception());
    }

     return client_context_.get_connection_manager().get_random_connection();
}

int64_t
sql_service::uuid_high(const boost::uuids::uuid& uuid)
{
    return boost::endian::load_big_s64(uuid.begin());
}

int64_t
sql_service::uuid_low(const boost::uuids::uuid& uuid)
{
    return boost::endian::load_big_s64(uuid.begin() + 8);
}

void sql_service::rethrow(std::exception_ptr exc_ptr, const exception::iexception &ie) {
    // Make sure that access_control is thrown as a top-level exception
    try {
        std::rethrow_if_nested(exc_ptr);
    } catch (exception::access_control &) {
        throw;
    } catch (...) {
        impl::query_utils::throw_public_exception(exc_ptr, ie, client_id());
    }
}

    boost::uuids::uuid sql_service::client_id() {
        return client_context_.get_connection_manager().get_client_uuid();
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

    sql_ = sql_string;

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

sql_column_metadata::sql_column_metadata(std::string name,
                                         sql_column_type type,
                                         bool nullable)
  : name_{ std::move(name) }
  , type_{ type }
  , nullable_{ nullable }
{
}

const std::string&
sql_column_metadata::name() const
{
    return name_;
}

sql_column_type
sql_column_metadata::type() const
{
    return type_;
}

bool
sql_column_metadata::nullable() const
{
    return nullable_;
}

    hazelcast_sql_exception::hazelcast_sql_exception(boost::uuids::uuid originating_member_id,
                                                     int32_t code,
                                                     const boost::optional<std::string>& message,
                                                     const boost::optional<std::string>& suggestion,
                                                     std::exception_ptr cause)
            : hazelcast_("",message ? message.value() : "", "", cause)
            , originating_member_id_(originating_member_id)
            , code_(code)
            , suggestion_(suggestion)
    {}

    const boost::uuids::uuid &hazelcast_sql_exception::originating_member_id() const {
        return originating_member_id_;
    }

    int32_t hazelcast_sql_exception::code() const {
        return code_;
    }

    const boost::optional<std::string> &hazelcast_sql_exception::suggestion() const {
        return suggestion_;
    }

namespace impl {

query_id::query_id(int64_t member_id_high,
                   int64_t member_id_low,
                   int64_t local_id_high,
                   int64_t local_id_low)
  : member_id_high_{ member_id_high }
  , member_id_low_{ member_id_low }
  , local_id_high_{ local_id_high }
  , local_id_low_{ local_id_low }
{
}

int64_t
query_id::member_id_high() const
{
    return member_id_high_;
}

int64_t
query_id::member_id_low() const
{
    return member_id_low_;
}

int64_t
query_id::local_id_high() const
{
    return local_id_high_;
}

int64_t
query_id::local_id_low() const
{
    return local_id_low_;
}

void query_utils::throw_public_exception(std::exception_ptr e, const exception::iexception &ie, boost::uuids::uuid id) {
    try {
       std::rethrow_exception(e);
    } catch (hazelcast_sql_exception &e) {
        throw;
    } catch (exception::query &e) {
        auto originating_member_id = e.originating_member_uuid();
        if (originating_member_id.is_nil()) {
            originating_member_id = id;
        }

        throw hazelcast_sql_exception(originating_member_id, e.code(), e.get_message(), e.suggestion());
    } catch (...) {
        throw hazelcast_sql_exception(id, static_cast<int32_t>(sql_error_code::GENERIC), ie.get_message(), boost::none, e);
    }
}

} // namespace impl

sql_result::sql_result(
  int64_t update_count,
  boost::optional<std::vector<sql_column_metadata>> row_metadata,
  boost::optional<impl::sql_page> first_page)
  : update_count_(update_count)
  , row_metadata_(std::move(row_metadata))
  , current_page_(std::move(first_page))
{
}

int64_t
sql_result::update_count() const
{
    return update_count_;
}

bool
sql_result::is_row_set() const
{
    return update_count() == -1;
}

boost::future<std::vector<sql_row>>
sql_result::fetch_page()
{
    std::vector<sql_row> rows;

    for (std::size_t i = 0; i < current_page_->columns()[0].size(); i++) {
        rows.push_back(sql_row{ i, current_page_.value() });
    }

    return boost::make_ready_future(std::move(rows));
}

bool
sql_result::has_more() const
{
    return current_page_ && !current_page_->last();
}

const boost::optional<std::vector<sql_column_metadata>>&
sql_result::row_metadata() const
{
    return row_metadata_;
}

} // namespace sql
} // namespace client
} // namespace hazelcast
