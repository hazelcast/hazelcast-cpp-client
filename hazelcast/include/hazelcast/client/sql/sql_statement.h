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

/**
 * Definition of an SQL statement.
 * <p>
 * This object is mutable. Properties are read once before the execution is
 * started. Changes to properties do not affect the behavior of already running
 * statements.
 */
class HAZELCAST_API sql_statement
{
public:
    /**
     * Value for the timeout that is not set. The value of
     * sql_config::statement_timeout_millis will be used.
     */
    static constexpr std::chrono::milliseconds TIMEOUT_NOT_SET{ -1 };

    /**
     * Value for the timeout that is disabled, meaning there's no time limit to
     * run a query.
     */
    static constexpr std::chrono::milliseconds TIMEOUT_DISABLED{ 0 };

    /** Default timeout. */
    static constexpr std::chrono::milliseconds DEFAULT_TIMEOUT =
      TIMEOUT_NOT_SET;

    /** Default cursor buffer size. */
    static constexpr int32_t DEFAULT_CURSOR_BUFFER_SIZE = 4096;

    /**
     * Creates a statement with the given query.
     * @param client The hazelcast client to be used for the statement.
     * @param query The query string.
     */
    sql_statement(hazelcast_client& client, std::string query);

    /**
     *
     * @return The sql string to be executed
     */
    const std::string& sql() const;

    /**
     * Sets the SQL string to be executed.
     * <p>
     * The SQL string cannot be empty.
     *
     * @param sql_string SQL string
     * @return this instance for chaining
     * @throws hazelcast::client::exception::illegal_argument if passed SQL
     * string is empty
     */
    sql_statement& sql(std::string sql_string);

    /**
     * Sets the values for statement parameters.
     * <p>
     * You may define parameter placeholders in the statement with the {@code
     * "?"} character. For every placeholder, a value must be provided. <p> When
     * the method is called, the contents of the list are copied. Subsequent
     * changes to the original list don't change the statement parameters.
     *
     * @param value the first statement parameter
     * @param other_params the other statement parameters if exist
     * @return this instance for chaining
     *
     * @see template<typename Param> add_parameter(const Param& value)
     * @see clear_parameters()
     */
    template<typename... Param>
    sql_statement& set_parameters(Param... params);

    /**
     * Adds a single parameter value to the end of the parameter values list.
     *
     * @param value parameter value
     * @return this instance for chaining
     *
     * @see set_parameters()
     * @see clear_parameters()
     */
    template<typename Param>
    sql_statement& add_parameter(const Param& value);

    /**
     * Clears statement parameter values.
     *
     * @return this instance for chaining
     *
     * @see set_parameters()
     * @see template<typename Param> add_parameter(const Param& value)
     */
    sql_statement& clear_parameters();

    /**
     * Gets the cursor buffer size (measured in the number of rows).
     *
     * @return cursor buffer size (measured in the number of rows)
     */
    int32_t cursor_buffer_size() const;

    /**
     * Sets the cursor buffer size (measured in the number of rows).
     * <p>
     * When a statement is submitted for execution, a sql_result is returned as
     * a result. When rows are ready to be consumed, they are put into an
     * internal buffer of the cursor. This parameter defines the maximum number
     * of rows in that buffer. When the threshold is reached, the backpressure
     * mechanism will slow down the execution, possibly to a complete halt, to
     * prevent out-of-memory. <p> Only positive values are allowed. <p> The
     * default value is expected to work well for most workloads. A bigger
     * buffer size may give you a slight performance boost for queries with
     * large result sets at the cost of increased memory consumption. <p>
     * Defaults to sql_statement::DEFAULT_CURSOR_BUFFER_SIZE.
     *
     * @param size cursor buffer size (measured in the number of rows)
     * @return this instance for chaining
     *
     * @see hazelcast::client::sql::sql_service::execute(const sql_statement&
     * statement)
     * @see hazelcast::client::sql::sql_result
     */
    sql_statement& cursor_buffer_size(int32_t size);

    /**
     * Gets the execution timeout in milliseconds.
     *
     * @return execution timeout in milliseconds
     */
    std::chrono::milliseconds timeout() const;

    /**
     * Sets the execution timeout in milliseconds.
     * <p>
     * If the timeout is reached for a running statement, it will be cancelled
     * forcefully. <p> Zero value means no timeout.
     * sql_statement::TIMEOUT_NOT_SET means that the value from
     * sql_config::statement_timeout() will be used. Other negative values are
     * prohibited. <p> Defaults to sql_statement::TIMEOUT_NOT_SET .
     *
     * @param timeout execution timeout in milliseconds, \c0 for no timeout,
     * \c-1 to user member's default timeout
     * @return this instance for chaining
     *
     * @see sql_config::statement_timeout()
     */
    sql_statement& timeout(std::chrono::milliseconds timeout);

    /**
     * Gets the expected result type.
     *
     * @return expected result type
     */
    sql_expected_result_type expected_result_type() const;

    /**
     * Sets the expected result type.
     *
     * @param type expected result type
     * @return this instance for chaining
     */
    sql_statement& expected_result_type(sql_expected_result_type type);

    /**
     * Gets the schema name.
     *
     * @return the schema name or \code{.cpp}boost::none\endcode if there is
     * none
     */
    const boost::optional<std::string>& schema() const;

    /**
     * Sets the schema name. The engine will try to resolve the non-qualified
     * object identifiers from the statement in the given schema. If not found,
     * the default search path will be used, which looks for objects in the
     * predefined schemas \code{.unparsed}"partitioned"\endcode and
     * \code{.unparsed}"public"\endcode. <p> The schema name is case sensitive.
     * For example, \code{.cpp}"foo"\endcode and \code{.cpp}"Foo"\endcode are
     * different schemas. <p> The default value is
     * \code{.cpp}boost::none\endcode meaning only the default search path is
     * used.
     *
     * @param schema the current schema name
     * @return this instance for chaining
     */
    sql_statement& schema(boost::optional<std::string> schema);

private:
    using data = serialization::pimpl::data;
    using serialization_service = serialization::pimpl::SerializationService;

    sql_statement(spi::ClientContext& client_context, std::string query);

    std::string sql_;
    std::vector<data> serialized_parameters_;
    int32_t cursor_buffer_size_;
    std::chrono::milliseconds timeout_;
    sql::sql_expected_result_type expected_result_type_;
    boost::optional<std::string> schema_;

    serialization_service& serialization_service_;

    friend hazelcast::client::sql::sql_service;
};

template<typename Param>
sql_statement&
sql_statement::add_parameter(const Param& param)
{
    serialized_parameters_.emplace_back(serialization_service_.to_data(param));

    return *this;
}

template<typename... Param>
sql_statement&
sql_statement::set_parameters(Param... params)
{
    int _[] = { 0, ((void)add_parameter(params), 0)... };
    (void)_;
    return *this;
}

} // namespace sql
} // namespace client
} // namespace hazelcast
