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

// This class is NOT thread-safe. Do NOT use simultaneously from multiple
// threads.
/**
 * SQL query result. Depending on the statement type it represents a pages of
 * rows or an update count.
 * <p>
 * <h4>Usage for page of rows</h4>
 *
 * <ol>
 *     <li>Use page_iterator() to iterate the pages. see page_iterator_type</li>
 *     <li>Use close() to release the resources associated with the result.</li>
 * </ol>

* <p>
  * Code example:
  * <pre>
  * auto result = hz.get_sql().execute("SELECT * FROM person");
  * for (auto it = result.page_iterator(); it; (++it).get()) {
  *    for (auto const &row : (*it).rows()) {
  *       // Process the row.
  *    }
  * }
 * </pre>
 *
 * <h4>Usage for update count</h4>
 *
 * <pre>
 *     auto updated = hz.get_sql().execute("UPDATE ...").get().update_count();
 * </pre>
 *
  * You don't need to call close() in this case.
 */
class HAZELCAST_API sql_result
{
public:
    class HAZELCAST_API page_iterator_type
    {
    public:
        page_iterator_type(sql_result* result, boost::optional<sql_page> page);

        boost::future<void> operator++();

        const boost::optional<sql_page>& operator*() const;

        operator bool() const;

    private:
        sql_result* result_;
        boost::optional<sql_page> page_;
    };

    sql_result(
      spi::ClientContext* client_context,
      sql_service* service,
      std::shared_ptr<connection::Connection> connection,
      impl::query_id id,
      int64_t update_count,
      boost::optional<std::vector<sql_column_metadata>> columns_metadata,
      boost::optional<sql_page> first_page,
      int32_t cursor_buffer_size);

    sql_result();

    /**
     * Return whether this result has rows to iterate using the page_iterator()
     * method.
     */
    bool row_set() const;

    /**
     * Gets the row metadata.
     *
     * @returns the metadata of the rows in this result.
     *
     * @throws illegal_state_exception if this result doesn't have rows, but
     * only an update count
     */
    const sql_row_metadata& row_metadata() const;

    /**
     * Returns the number of rows updated by the statement or -1 if this result
     * is a row set. In case the result doesn't contain rows but the update
     * count isn't applicable or known, 0 is returned.
     */
    int64_t update_count() const;

    /**
     * Release the resources associated with the query result.
     * <p>
     * The query engine delivers the rows asynchronously. The query may become
     * inactive even before all rows are consumed. The invocation of this
     * command will cancel the execution of the query on all members if the
     * query is still active. Otherwise it is no-op. For a result with an update
     * count it is always no-op.
     */
    boost::future<void> close();

    /**
     * Returns an iterator over the result pages.
     *
     * The iterator may be requested only once.
     *
     * @return the iterator to be used over the result.
     * The iterator iterates page by page over the result.
     *
     * @throws exception::illegal_state if the iterator is requested more than
     * once or if this result does not have any pages.
     *
     */
    page_iterator_type page_iterator();

private:
    friend class sql_service;

    spi::ClientContext* client_context_;
    sql_service* service_;
    std::shared_ptr<connection::Connection> connection_;
    impl::query_id query_id_;
    int64_t update_count_;
    boost::optional<sql_row_metadata> row_metadata_;
    boost::optional<sql_page> first_page_;

    bool iterator_requested_;

    /** Whether the result is closed. When true, there is no need to send the
     * "cancel" request to the server. */
    bool closed_;

    int32_t cursor_buffer_size_;

    boost::future<sql_page> fetch_page();

    template<typename T>
    boost::optional<T> to_object(serialization::pimpl::data data)
    {
        return client_context_->get_serialization_service().to_object<T>(data);
    }
};

} // namespace sql
} // namespace client
} // namespace hazelcast
