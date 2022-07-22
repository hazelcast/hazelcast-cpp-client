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

    auto query_conn =
      client_context_.get_connection_manager().get_random_connection();

    if (!query_conn) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_client_not_active(
          "hazelcast_client_instance_impl::execute",
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

sql_statement&
sql_statement::add_parameter()
{
    return *this;
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
