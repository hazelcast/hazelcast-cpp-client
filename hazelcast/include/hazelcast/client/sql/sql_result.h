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

namespace hazelcast {
namespace client {
namespace sql {
// This class is NOT thread-safe. Do NOT use simultaneously from multiple threads.
class HAZELCAST_API sql_result
{
public:
    class page_iterator_type {
    public:
        page_iterator_type(sql_result &result, boost::optional<sql_page> page);

        boost::future<page_iterator_type> operator++();

        const boost::optional<sql_page> &operator*();

    private:
        sql_result &result_;
        const boost::optional<sql_page> page_;
    };

    sql_result(int64_t update_count,
           boost::optional<std::vector<sql_column_metadata>> row_metadata,
           boost::optional<sql_page> first_page,
           boost::optional<bool> is_inifinite_rows);

    /**
     * Return whether this result has rows to iterate using the \page_iterator() method.
     */
    bool is_row_set() const;

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
    int64_t update_count_;
    boost::optional<std::vector<sql_column_metadata>> row_metadata_;
    boost::optional<sql_page> first_page_;

    /** Whether the result set is unbounded. */
    boost::optional<bool> is_infinite_rows_;

    bool iterator_requested_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
