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
#pragma once

#include <chrono>
#include <cstddef>
#include <string>

#include <boost/optional/optional.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/sql/sql_expected_result_type.h"

namespace hazelcast {
namespace client {
namespace sql {
class sql_service;

class HAZELCAST_API sql_statement
{
public:
    sql_statement(hazelcast_client& client, std::string query);

    const std::string& query() const;

    template<typename Param>
    void add_parameter(const Param& param);

    std::size_t cursor_buffer_size() const;
    void cursor_buffer_size(std::size_t size);

    std::chrono::milliseconds timeout() const;
    void timeout(std::chrono::milliseconds timeout);

    sql_expected_result_type expected_result_type() const;
    void expected_result_type(sql_expected_result_type type);

    const boost::optional<std::string>& schema() const;
    void schema(boost::optional<std::string> schema);

private:
    using data = serialization::pimpl::data;
    using serialization_service = serialization::pimpl::SerializationService;

    sql_statement(spi::ClientContext& client_context, std::string query);

    std::string query_;
    std::vector<data> serialized_parameters_;
    std::size_t cursor_buffer_size_;
    std::chrono::milliseconds timeout_;
    sql::sql_expected_result_type expected_result_type_;
    boost::optional<std::string> schema_;

    serialization_service& serialization_service_;

    friend hazelcast::client::sql::sql_service;
};

template<typename Param>
void
sql_statement::add_parameter(const Param& param)
{
    serialized_parameters_.emplace_back(serialization_service_.to_data(param));
}

} // namespace sql
} // namespace client
} // namespace hazelcast
