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

namespace hazelcast {
namespace client {
namespace sql {

class HAZELCAST_API sql_row
{
public:
    template<typename T>
    const boost::optional<T>& get_value(std::size_t column) const
    {
        return page_.template get_value<T>(column, id_);
    }

    std::size_t id_;
    impl::sql_page& page_;
};

// TODO multi-page support
class HAZELCAST_API sql_result
{
public:
    sql_result(int64_t update_count,
           boost::optional<std::vector<sql_column_metadata>> row_metadata,
           boost::optional<impl::sql_page> first_page,
           boost::optional<bool> is_inifinite_rows);

    /**
     * Returns the number of rows updated by the statement or -1 if this result
     * is a row set. In case the result doesn't contain rows but the update
     * count isn't applicable or known, 0 is returned.
     */
    int64_t update_count() const;

    bool is_row_set() const;

    boost::future<std::vector<sql_row>> fetch_page();

    bool has_more() const;

    const boost::optional<std::vector<sql_column_metadata>>& row_metadata() const;

    boost::optional<bool> is_infinite_rows() const;

private:
    int64_t update_count_;
    boost::optional<std::vector<sql_column_metadata>> row_metadata_;
    boost::optional<impl::sql_page> current_page_;
    boost::optional<bool> is_infinite_rows_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
