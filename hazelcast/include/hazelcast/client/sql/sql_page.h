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

#include <vector>
#include <boost/any.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_type.h"

namespace hazelcast {
namespace client {
namespace sql {

class HAZELCAST_API sql_page
{
    using column = std::vector<boost::any>;

public:
    class HAZELCAST_API sql_row
    {
    public:
        sql_row(size_t row_index, const sql_page &page,
                boost::optional<std::vector<sql_column_metadata>> &row_metadata);

        /**
         * Gets the value of the column by index.
         * <p>
         * The class of the returned value depends on the SQL type of the column. No implicit conversions are performed on the value.
         *
         * @param column_index column index, zero-based.
         * @return value of the column
         *
         * @throws hazelcast::client::exception::index_out_of_bounds_exception if the column index is out of bounds
         * @throws boost::bad_any_cast if the type of the column type isn't assignable to the type \codeT\endcode
         *
         */
        template<typename T>
        const boost::optional<T>& get_object(std::size_t column_index) const
        {
            return page_.get_column_value<T>(column_index, row_index_);
        }

        /**
         * Gets the value of the column by column name.
         * <p>
         * Column name should be one of those defined in row_metadata, case-sensitive.
         * <p>
         * The class of the returned value depends on the SQL type of the column. No implicit conversions are performed on the value.
         *
         * @param column_name column name
         * @return value of the column
         *
         * @throws hazelcast::client::exception::illegal_argument if a column with the given name is not found
         * @throws boost::any_cast_exception if the type of the column type isn't assignable to the type \codeT\endcode
         *
         * @see sql_column_metadata#name()
         * @see sql_column_metadata#type()
         */
        template<typename T>
        const boost::optional<T>& get_object(const std::string &column_name) const
        {
            return page_.get_column_value<T>(column_index, row_index_);
        }

        /**
         * Gets the row metadata.
         * @returns vector of column metadata, and returns boost::none if the result contains only an update count.
         *
         */
        const boost::optional<std::vector<sql_column_metadata>>& row_metadata() const;

    private:
        const std::size_t row_index_;
        const sql_page& page_;
        boost::optional<std::vector<sql_column_metadata>> &row_metadata_;
    };

    sql_page(std::vector<sql_column_type> column_types,
         std::vector<column> columns,
         bool last,
         boost::optional<std::vector<sql_column_metadata>> &row_metadata);

    template<typename T>
    T get_column_value(std::size_t column_index, std::size_t row_index) const {
        assert(column_index < column_count());
        assert(row_index < row_count());

        return boost::any_cast<T>(columns_[column_index][row_index]);
    }

    const std::vector<sql_column_type>& column_types() const;

    bool last() const;

    std::size_t column_count() const;

    std::size_t row_count() const;

    const std::vector<sql_row> &rows() const;
private:
    const std::vector<sql_column_type> column_types_;
    const std::vector<column> columns_;
    std::vector<sql_row> rows_;
    const bool last_;
    boost::optional<std::vector<sql_column_metadata>> &row_metadata_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
