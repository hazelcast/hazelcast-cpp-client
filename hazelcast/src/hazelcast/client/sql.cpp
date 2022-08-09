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

    sql::impl::query_id qid{member_id, local_id};

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
      [this, query_conn](boost::future<ClientMessage> response_fut) {
          try {
              auto response = response_fut.get();
              return handle_execute_response(response);
          } catch (exception::iexception &e) {
              rethrow(std::current_exception(), e);
          }

      });

    return result_fut;
}

    sql_service::sql_execute_response_parameters sql_service::decode_response(protocol::ClientMessage &msg) const {
        static constexpr size_t RESPONSE_UPDATE_COUNT_FIELD_OFFSET = protocol::ClientMessage::RESPONSE_HEADER_LEN;
        static constexpr size_t RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET = RESPONSE_UPDATE_COUNT_FIELD_OFFSET + protocol::ClientMessage::INT64_SIZE;

        sql_execute_response_parameters response;

        auto initial_frame_header = msg.read_frame_header();
        msg.rd_ptr(protocol::ClientMessage::RESPONSE_HEADER_LEN -
                        protocol::ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS);

        response.update_count = msg.get<int64_t>();

        if (initial_frame_header.frame_len >= RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET + protocol::ClientMessage::INT8_SIZE) {
            response.is_infinite_rows = msg.get<bool>();
            response.is_infinite_rows_exist = true;
            // skip initial_frame
            msg.rd_ptr(static_cast<int32_t>(initial_frame_header.frame_len) -
                       (RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET +
                        protocol::ClientMessage::INT8_SIZE));
        } else {
            response.is_infinite_rows_exist = false;
            // skip initial_frame
            msg.rd_ptr(static_cast<int32_t>(initial_frame_header.frame_len) -
                       RESPONSE_IS_INFINITE_ROWS_FIELD_OFFSET);
        }

        response.row_metadata =
                msg.get_nullable<std::vector<sql_column_metadata>>();
        response.first_page = msg.get_nullable<impl::sql_page>();
        response.error = msg.get_nullable<impl::sql_error>();
        
        return response;
    }

sql_result sql_service::handle_execute_response(protocol::ClientMessage &msg) {
    auto response = decode_response(msg);
    if (response.error) {
        BOOST_THROW_EXCEPTION(
                sql::hazelcast_sql_exception(response.error->originating_member_id,
                                             response.error->code,
                                             response.error->message,
                                             response.error->suggestion));
    }

    return sql_result{ response.update_count,
                       std::move(response.row_metadata),
                       std::move(response.first_page),
                       response.is_infinite_rows_exist ? boost::make_optional(response.is_infinite_rows) : boost::none};
}

std::shared_ptr<connection::Connection> sql_service::query_connection() {
    try {
        auto &cs = client_context_.get_client_cluster_service();
        auto members = cs.get_member_list();
        auto connection = client_context_.get_connection_manager().connection_for_sql([&]() {
            return impl::query_utils::member_of_same_larger_version_group(members);
        }, [&](boost::uuids::uuid id) {
            return cs.get_member(id);
        });

        if (!connection) {
            exception::query e(static_cast<int32_t>(impl::sql_error_code::CONNECTION_PROBLEM), "Client is not connected");
            rethrow(std::make_exception_ptr(e), e);
        }

        return connection;
    } catch (exception::iexception &e) {
        rethrow(std::current_exception(), e);
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
        impl::query_utils::throw_public_exception(std::current_exception(), ie, client_id());
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

    boost::optional<member> query_utils::member_of_same_larger_version_group(const std::vector<member> &members) {
        // The members should have at most 2 different version (ignoring the patch version).
        // Find a random member from the larger same-version group.

        // we don't use 2-element array to save on litter
        boost::optional<member::version> version0;
        boost::optional<member::version> version1;
        size_t count0 = 0;
        size_t count1 = 0;
        size_t gross_majority = members.size() / 2;

        for (const auto &m : members) {
            if (m.is_lite_member()) {
                continue;
            }
            auto v = m.get_version();
            size_t current_count = 0;
            if (!version0 || *version0 == v ) {
                version0 = v;
                current_count = ++count0;
            } else if (!version1 || *version1 == v) {
                version1 = v;
                current_count = ++count1;
            } else {
                throw exception::runtime("query_utils::member_of_same_larger_version_group",
                                         (boost::format("More than 2 distinct member versions found: %1% , %2%")  %version0 %version1).str());
            }
        }

        assert(count1 == 0 || count0 > 0);

        // no data members
        if (count0 == 0) {
            return boost::none;
        }

        size_t count;
        member::version version{};
        if (count0 > count1 || (count0 == count1 && *version0 > *version1)) {
            count = count0;
            version = *version0;
        } else {
            count = count1;
            version = *version1;
        }

        // otherwise return a random member from the larger group
        static thread_local std::mt19937 generator;
        std::uniform_int_distribution<int> distribution(0, count);
        auto random_member_index = distribution(generator);
        for (const auto &m : members) {
            if (!m.is_lite_member() && m.get_version() == version) {
                random_member_index--;
                if (random_member_index < 0) {
                    return m;
                }
            }
        }

        throw exception::runtime("query_utils::member_of_same_larger_version_group", "should never get here");
    }

} // namespace impl

sql_result::sql_result(
  int64_t update_count,
  boost::optional<std::vector<sql_column_metadata>> row_metadata,
  boost::optional<impl::sql_page> first_page,
  boost::optional<bool> is_infinite_rows)
  : update_count_(update_count)
  , row_metadata_(std::move(row_metadata))
  , current_page_(std::move(first_page))
  , is_infinite_rows_(is_infinite_rows)
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

    boost::optional<bool> sql_result::is_infinite_rows() const {
        return is_infinite_rows_;
    }

} // namespace sql
} // namespace client
} // namespace hazelcast
