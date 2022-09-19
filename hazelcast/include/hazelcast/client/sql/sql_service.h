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
 * the <em>CREATE MAPPING</em> command. <p> When a query is executed, an {@link
 * sql_result} is returned. You may get row iterator from the result. The result
 * must be closed at the end. The code snippet below demonstrates a typical
 * usage pattern: <pre>
 *     HazelcastInstance instance = ...;
 *
 *     try (SqlResult result = instance.sql().execute("SELECT * FROM person")) {
 *         for (SqlRow row : result) {
 *             long personId = row.getObject("personId");
 *             String name = row.getObject("name");
 *             ...
 *         }
 *     }
 * </pre>
 */
class HAZELCAST_API sql_service
{
public:
    template<typename... Params>
    boost::future<sql_result> execute(const std::string& query,
                                      const Params&... params)
    {
        sql_statement s{ client_context_, query };
        int _[] = { 0, (s.add_parameter(params), 0)... };
        (void)_;
        return execute(s);
    }

    boost::future<sql_result> execute(const sql_statement& statement);

    /**
     * Close remote query cursor.
     *
     * @param connection Connection.
     * @param id    Query ID.
     */
    boost::future<void> close(
      const std::shared_ptr<connection::Connection>& connection,
      impl::query_id id);

private:
    friend client::impl::hazelcast_client_instance_impl;
    friend sql_result;

    client::spi::ClientContext& client_context_;

    struct sql_execute_response_parameters
    {
        int64_t update_count;
        boost::optional<std::vector<sql_column_metadata>> row_metadata;
        boost::optional<sql_page> first_page;
        boost::optional<impl::sql_error> error;
        bool is_infinite_rows = false;
        bool is_infinite_rows_exist = false;
    };

    struct sql_fetch_response_parameters
    {
        boost::optional<sql_page> page;
        boost::optional<impl::sql_error> error;
    };

    explicit sql_service(client::spi::ClientContext& context);

    static int64_t uuid_high(const boost::uuids::uuid& uuid);
    static int64_t uuid_low(const boost::uuids::uuid& uuid);

    std::shared_ptr<connection::Connection> query_connection();

    void rethrow(const std::exception_ptr& exc_ptr);
    void rethrow(std::exception_ptr cause_ptr,
                 const std::shared_ptr<connection::Connection>& connection);

    boost::uuids::uuid client_id();

    sql_result handle_execute_response(
      protocol::ClientMessage& msg,
      std::shared_ptr<connection::Connection> connection,
      impl::query_id id,
      int32_t cursor_buffer_size);

    static sql_execute_response_parameters decode_execute_response(
      protocol::ClientMessage& msg);

    impl::query_id create_query_id(
      const std::shared_ptr<connection::Connection>& query_conn);

    boost::future<sql_page> fetch_page(
      const impl::query_id& q_id,
      int32_t cursor_buffer_size,
      const std::shared_ptr<connection::Connection>& connection);

    static sql_fetch_response_parameters decode_fetch_response(
      protocol::ClientMessage message);

    static void handle_fetch_response_error(
      boost::optional<impl::sql_error> error);
};
} // namespace sql
} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
