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
#include "hazelcast/client/sql/column_metadata.h"
#include "hazelcast/client/sql/column_type.h"
#include "hazelcast/client/sql/result.h"
#include "hazelcast/client/sql/statement.h"

namespace hazelcast {
namespace client {
namespace sql {

statement::statement(hazelcast_client& client, std::string query)
  : query_{ std::move(query) }
  , serialized_parameters_{}
  , cursor_buffer_size_{ 4096 }
  , timeout_{ -1 }
  , expected_result_type_{ expected_result_type::any }
  , schema_{}
  , serialization_service_{
      spi::ClientContext(client).get_serialization_service()
  }
{}

const std::string&
statement::query() const
{
    return query_;
}

std::size_t
statement::cursor_buffer_size() const
{
    return cursor_buffer_size_;
}

std::chrono::milliseconds
statement::timeout() const
{
    return timeout_;
}

void
statement::timeout(std::chrono::milliseconds timeout)
{
    timeout_ = timeout;
}

void
statement::cursor_buffer_size(std::size_t s)
{
    cursor_buffer_size_ = s;
}

const boost::optional<std::string>&
statement::schema() const
{
    return schema_;
}

void
statement::schema(boost::optional<std::string> schema)
{
    schema_ = std::move(schema);
}

sql::expected_result_type
statement::expected_result_type() const
{
    return expected_result_type_;
}

void
statement::expected_result_type(sql::expected_result_type type)
{
    expected_result_type_ = type;
}

column_metadata::column_metadata(std::string name,
                                 column_type type,
                                 bool nullable)
  : name_{ std::move(name) }
  , type_{ type }
  , nullable_{ nullable }
{}

const std::string&
column_metadata::name() const
{
    return name_;
}

column_type
column_metadata::type() const
{
    return type_;
}

bool
column_metadata::nullable() const
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
{}

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

result::result(int64_t update_count,
               boost::optional<std::vector<column_metadata>> row_metadata,
               boost::optional<impl::page> first_page)
  : update_count_(update_count)
  , row_metadata_(std::move(row_metadata))
  , current_page_(std::move(first_page))
{}

int64_t
result::update_count() const
{
    return update_count_;
}

bool
result::is_row_set() const
{
    return update_count() == -1;
}

boost::future<std::vector<row>>
result::fetch_page()
{
    std::vector<row> rows;

    for (std::size_t i = 0; i < current_page_->columns()[0].size(); i++) {
        rows.push_back(row{ i, current_page_.value() });
    }

    return boost::make_ready_future(std::move(rows));
}

bool
result::has_more() const
{
    return current_page_ && !current_page_->last();
}

const boost::optional<std::vector<column_metadata>>&
result::row_metadata() const
{
    return row_metadata_;
}

} // namespace sql
} // namespace client
} // namespace hazelcast
