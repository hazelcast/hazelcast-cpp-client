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
#pragma once

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_result.h"
#include "hazelcast/client/sql/sql_statement.h"
#include "hazelcast/client/sql/hazelcast_sql_exception.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace impl {
class hazelcast_client_instance_impl;
}
class hazelcast_client;
namespace sql {

/**
 * A service to execute SQL statements.
 * <p>
 * In order to use the service, Jet engine must be enabled on the server side -
 * SQL statements are executed as Jet jobs. On members, the {@code
 * hazelcast-sql.jar} must be on the classpath, otherwise an exception will be
 * thrown; on client, it is not necessary.
 *
 * <p>
 * <h1>Overview</h1>
 * Hazelcast is currently able to execute distributed SQL queries using the
 * following connectors:
 * <ul>
 *  <li>IMap
 *  <li>Kafka
 *  <li>Files
 * </ul>
 *
 * When an SQL statement is submitted to a member, it is parsed and optimized
 * hazelcast-sql module, that is based on <a
 * href="https://calcite.apache.org">Apache Calcite</a>. During optimization a
 * statement is converted into a directed acyclic graph (DAG) that is sent to
 * cluster members for execution. Results are sent back to the originating
 * member asynchronously and returned to the user via {@link sql_result}. <p>
 * SQL statements are not atomic. <em>INSERT</em>/<em>SINK</em> can fail and
 * commit part of the data. <p> <h1>Usage</h1> Before you can access any object
 * using SQL, a <em>mapping</em> has to be created. See the reference manual for
 * the <em>CREATE MAPPING</em> command. <p> When a query is executed, an
 * sql_result is returned. You may get row iterator from the result. The result
 * must be closed at the end. The code snippet below demonstrates a typical
 * usage pattern: <pre>
 *     auto hz = hazelcast::new_client().get();
 *
 *     try {
 *         // Get the SQL service from the client and execute a query
 *         auto result = hz.get_sql().execute("SELECT * FROM person").get();
 *         for (auto it = result.page_iterator(); it; (++it).get()) {
 *              // iterate over the rows in the page
 *              for (auto const &row : (*it).rows()) {
 *                  auto person_id = row.get<int64_t>("personId");
 *                  auto name = row.get<std::string>("name");
 *                  ...
 *              }
 *         }
 *
 *         // Close the result when done.
 *         result.close().get();
 *     } catch (hazelcast::client::exception &e) {
 *          std::cerr << "Query failed: " << e.what() << std::endl;
 *     }
 * </pre>
 */
class HAZELCAST_API sql_service
{
public:
    /**
     * Convenient method to execute a distributed query with the given
     * parameter values.
     * <p>
     * Converts passed SQL string and parameter values into an sql_statement
     * object and invokes execute(const sql_statement& statement).
     *
     * @param sql       SQL string
     * @param arguments query parameter values that will be passed to
     * sql_statement::add_parameter(const Param& param)
     * @return result of the execution
     *
     * @throws illegal_argument if the SQL string is empty
     * @throws hazelcast_sql_exception in case of execution error
     * @see sql_service
     * @see sql_statement
     * @see execute(const sql_statement& statement)
     */
    template<typename... Params>
    boost::future<std::shared_ptr<sql_result>> execute(const std::string& query,
                                      const Params&... params)
    {
        sql_statement s{ client_context_, query };
        int _[] = { 0, (s.add_parameter(params), 0)... };
        (void)_;
        return execute(s);
    }

    /**
     * Executes an SQL statement.
     *
     * @param statement statement to be executed
     * @return result of the execution
     * @throws hazelcast_sql_exception in case of execution error
     * @see sql_service
     */
    boost::future<std::shared_ptr<sql_result>> execute(
      const sql_statement& statement);

private:
    friend client::impl::hazelcast_client_instance_impl;
    friend sql_result;

    client::spi::ClientContext& client_context_;

    struct sql_execute_response_parameters
    {
        int64_t update_count;
        std::shared_ptr<sql_row_metadata> row_metadata;
        std::shared_ptr<sql_page> first_page;
        boost::optional<impl::sql_error> error;
        bool is_infinite_rows = false;
        bool is_infinite_rows_exist = false;
    };

    struct sql_fetch_response_parameters
    {
        boost::optional<std::shared_ptr<sql_page>> page;
        boost::optional<impl::sql_error> error;
    };

    explicit sql_service(client::spi::ClientContext& context);

    std::shared_ptr<connection::Connection> query_connection();

    void rethrow(const std::exception_ptr& exc_ptr);
    void rethrow(std::exception_ptr cause_ptr,
                 const std::shared_ptr<connection::Connection>& connection);

    boost::uuids::uuid client_id();

    std::shared_ptr<sql_result> handle_execute_response(
      protocol::ClientMessage& msg,
      std::shared_ptr<connection::Connection> connection,
      impl::query_id id,
      int32_t cursor_buffer_size);

    static sql_execute_response_parameters decode_execute_response(
      protocol::ClientMessage& msg);

    impl::query_id create_query_id(
      const std::shared_ptr<connection::Connection>& query_conn);

    boost::future<std::shared_ptr<sql_page>> fetch_page(
      const impl::query_id& q_id,
      int32_t cursor_buffer_size,
      const std::shared_ptr<connection::Connection>& connection);

    static sql_fetch_response_parameters decode_fetch_response(
      protocol::ClientMessage message);

    static void handle_fetch_response_error(
      boost::optional<impl::sql_error> error);

    /**
     * Close remote query cursor.
     *
     * @param connection Connection.
     * @param id    Query ID.
     */
    boost::future<void> close(
      const std::shared_ptr<connection::Connection>& connection,
      impl::query_id id);
};
} // namespace sql
} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
