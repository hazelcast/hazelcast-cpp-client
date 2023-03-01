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
 *     <li>Use iterator() to iterate the pages. see page_iterator</li>
 *     <li>Use close() to release the resources associated with the result.</li>
 * </ol>

* <p>
  * Code example:
  * <pre>
  * auto result = hz.get_sql().execute("SELECT * FROM person");
  * for (auto itr = result.iterator(); itr.has_next();) {
  *    auto page = itr.next().get();
  *
  *    for (auto const &row : page->rows()) {
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
class HAZELCAST_API sql_result : public std::enable_shared_from_this<sql_result>
{
public:
    class HAZELCAST_API page_iterator
    {
    public:
        page_iterator(std::shared_ptr<sql_result> result,
                      std::shared_ptr<sql_page> first_page);

        page_iterator(const page_iterator&) = delete;
        page_iterator(page_iterator&&) = default;
        page_iterator& operator=(const page_iterator&) = delete;
        page_iterator& operator=(page_iterator&&) = default;

        /**
         * Fetches the new page
         * @throws illegal_access if it is called before previous page is
         * already fetched.
         * @throws no_such_element if there are no more pages to be retrieved.
         * only an update count
         * @throws hazelcast_sql_exception if any other errors occurred.
         */
        boost::future<std::shared_ptr<sql_page>> next();

        /**
         * Tells whether there are pages to be retrieved
         */
        bool has_next() const;

    private:

        std::shared_ptr<std::atomic<bool>> in_progress_;
        std::shared_ptr<std::atomic<bool>> last_;
        std::shared_ptr<sql_row_metadata> row_metadata_;
        serialization::pimpl::SerializationService* serialization_;
        std::shared_ptr<sql_result> result_;
        std::shared_ptr<sql_page> first_page_;
    };

    /**
     * The destructor closes the result if it were open.
     */
    virtual ~sql_result();

    /**
     * Return whether this result has rows to iterate using the iterator()
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
    page_iterator iterator();

    /**
     * Returns partition argument index
     *     
     *
     * @return the partition argument index of sql result
     *
     */
    uint32_t get_partition_argument_index() {
        return partition_argument_index_;
    }    

private:
    friend class sql_service;

    spi::ClientContext* client_context_;
    sql_service* service_;
    std::shared_ptr<connection::Connection> connection_;
    impl::query_id query_id_;
    int64_t update_count_;
    std::shared_ptr<sql_row_metadata> row_metadata_;
    std::shared_ptr<sql_page> first_page_;
    int32_t partition_argument_index_;

    bool iterator_requested_;

    /** Whether the result is closed. When true, there is no need to send the
     * "cancel" request to the server. */
    std::atomic<bool> closed_;
    std::mutex mtx_;

    int32_t cursor_buffer_size_;

    /**
     * This is a PRIVATE API. Do NOT use it.
     *
     * @param client_context The client context object
     * @param service The sql service to be used
     * @param connection The connection used for the sql query
     * @param id The query id of the sql query
     * @param update_count The update count of the sql result
     * @param row_metadata The row metadata of the sql result
     * @param first_page The first page of the sql result
     * @param cursor_buffer_size The cursor buffer size of the sql result
     */
    sql_result(
      spi::ClientContext* client_context,
      sql_service* service,
      std::shared_ptr<connection::Connection> connection,
      impl::query_id id,
      int64_t update_count,
      std::shared_ptr<sql_row_metadata> row_metadata,
      std::shared_ptr<sql_page> first_page,
      int32_t cursor_buffer_size,
      int32_t partition_argument_index = -1);

private:
    boost::future<std::shared_ptr<sql_page>> fetch_page();

    template<typename T>
    boost::optional<T> to_object(serialization::pimpl::data data)
    {
        return client_context_->get_serialization_service().to_object<T>(data);
    }
    void check_closed() const;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
