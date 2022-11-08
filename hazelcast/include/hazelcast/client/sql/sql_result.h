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
class HAZELCAST_API sql_result : public std::enable_shared_from_this<sql_result>
{
public:
    class HAZELCAST_API page_iterator_type
    {
    public:
        page_iterator_type(std::shared_ptr<sql_result> result,
                           std::shared_ptr<sql_page> page);

        /**
         * Fetches the new page for the result.
         */
        boost::future<void> operator++();

        /**
         * Returns the current page the iterator points to.
         * @return the current page the iterator points to or boost::none if no
         * page exist.
         */
        std::shared_ptr<sql_page> operator*() const;

        /**
         * @return true if the iterator points to a page, false otherwise.
         */
        operator bool() const;

    private:
        std::shared_ptr<sql_result> result_;
        std::shared_ptr<sql_page> page_;
    };

    class HAZELCAST_API page_iterator
    {
    public:

        page_iterator(std::shared_ptr<sql_result> result, std::shared_ptr<sql_page> first_page)
            :   result_{move(result)}
            ,   first_page_{move(first_page)}
            ,   in_progress_{std::make_shared<std::atomic<bool>>()}
            ,   last_{std::make_shared<std::atomic<bool>>()}
        {}

        /**
         * Loads the new page for the result.
         */
        boost::future<std::shared_ptr<sql_page>> next()
        {
            if (first_page_)
            {
                auto page = move(first_page_);
                *last_ = page->last();
                return boost::make_ready_future<std::shared_ptr<sql_page>>(page);
            }

            if (*in_progress_){
                throw exception::illegal_access {
                    "Loading in progress."
                };
            }

            if (*last_)
                throw exception::illegal_state {
                    "Page is last page so next cannot be loaded."
                };

            *in_progress_ = true;

            auto page_future = result_->fetch_page();
            auto serialization_service = &result_->client_context_->get_serialization_service();
            auto row_metadata = result_->row_metadata_;

            std::weak_ptr<std::atomic<bool>> last_w { last_ };
            std::weak_ptr<std::atomic<bool>> in_progress_w { in_progress_ };

            return page_future.then(
                boost::launch::sync,
                [serialization_service , row_metadata, last_w, in_progress_w]
                (boost::future<std::shared_ptr<sql_page>> page_f) mutable 
                {
                    auto page = page_f.get();

                    page->serialization_service(serialization_service);
                    page->row_metadata(move(row_metadata));

                    auto last = last_w.lock();

                    if (last)
                        *last = page->last();

                    auto in_progress = in_progress_w.lock();

                    if (in_progress)
                        *in_progress = false;

                    return page;
                }
            );
        }

        bool has_next() const
        {
            return !*last_;
        }

    private:

        std::shared_ptr<std::atomic<bool>> in_progress_;
        std::shared_ptr<std::atomic<bool>> last_;
        std::shared_ptr<sql_result> result_;
        std::shared_ptr<sql_page> first_page_;
    };

    /**
     * The destructor closes the result if it were open.
     */
    virtual ~sql_result();

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
    page_iterator iterator()
    {
        check_closed();

        if (!first_page_) {
            throw exception::illegal_state(
            "sql_result::page_iterator",      
            "This result contains only update count");
        }

        if (iterator_requested_) {
            throw exception::illegal_state("sql_result::page_iterator",
                                        "Iterator can be requested only once");
        }

        iterator_requested_ = true;

        return page_iterator{ shared_from_this(), first_page_ };
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

    bool iterator_requested_;

    /** Whether the result is closed. When true, there is no need to send the
     * "cancel" request to the server. */
    bool closed_;

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
      int32_t cursor_buffer_size);

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
