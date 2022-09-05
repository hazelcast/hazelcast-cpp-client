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

#include <boost/thread/future.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/sql/sql_page.h"
#include "hazelcast/client/sql/sql_row_metadata.h"

namespace hazelcast {
namespace client {
namespace sql {
    class sql_service;

// This class is NOT thread-safe. Do NOT use simultaneously from multiple threads.
class HAZELCAST_API sql_result
{
public:
    class HAZELCAST_API page_iterator_type {
    public:
        page_iterator_type(sql_result *result, boost::optional<sql_page> page);

        boost::future<void> operator++();

        const boost::optional<sql_page> &operator*() const;

    private:
        sql_result *result_;
        boost::optional<sql_page> page_;
    };

    sql_result(spi::ClientContext *client_context, sql_service *service,
               std::shared_ptr<connection::Connection> connection,
               impl::query_id id, int64_t update_count,
           boost::optional<std::vector<sql_column_metadata>> columns_metadata,
           boost::optional<sql_page> first_page,
           boost::optional<bool> is_inifinite_rows, int32_t cursor_buffer_size);

    sql_result();

    /**
     * Return whether this result has rows to iterate using the \page_iterator() method.
     */
    bool is_row_set() const;

    /**
     * Gets the row metadata.
     *
     * @returns row metadata and boost::none if the result doesn't have rows, but
     *     only an update count
     */
    const boost::optional<sql_row_metadata> &row_metadata() const;

    /**
     * Returns the number of rows updated by the statement or -1 if this result
     * is a row set. In case the result doesn't contain rows but the update
     * count isn't applicable or known, 0 is returned.
     */
    int64_t update_count() const;

    /**
     * Release the resources associated with the query result.
     * <p>
     * The query engine delivers the rows asynchronously. The query may become inactive even before all rows are
     * consumed. The invocation of this command will cancel the execution of the query on all members if the query
     * is still active. Otherwise it is no-op. For a result with an update count it is always no-op.
     */
    boost::future<void> close();

    page_iterator_type page_iterator();

    boost::optional<bool> is_infinite_rows() const;

private:
    friend class sql_service;

    spi::ClientContext* client_context_;
    sql_service *service_;
    std::shared_ptr<connection::Connection> connection_;
    impl::query_id query_id_;
    int64_t update_count_;
    boost::optional<sql_row_metadata> row_metadata_;
    boost::optional<sql_page> first_page_;

    /** Whether the result set is unbounded. */
    boost::optional<bool> is_infinite_rows_;

    bool iterator_requested_;

    /** Whether the result is closed. When true, there is no need to send the "cancel" request to the server. */
    bool closed_;

    int32_t cursor_buffer_size_;

    boost::future<sql_page> fetch_page();
};

} // namespace sql
} // namespace client
} // namespace hazelcast
