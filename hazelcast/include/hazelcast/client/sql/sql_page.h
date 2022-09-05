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
#include <boost/optional.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_type.h"
#include "hazelcast/client/sql/sql_row_metadata.h"

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
        sql_row(size_t row_index, const sql_page *page, const sql_row_metadata *row_metadata);

        /**
         * Gets the value of the column by index.
         * <p>
         * The class of the returned value depends on the SQL type of the column. No implicit conversions are performed on the value.
         *
         * @param column_index column index, zero-based.
         * @return value of the column
         *
         * @throws hazelcast::client::exception::index_out_of_bounds if the column index is out of bounds
         * @throws boost::bad_any_cast if the type of the column type isn't assignable to the type \codeT\endcode
         *
         */
        template<typename T>
        boost::optional<T> get_object(std::size_t column_index) const
        {
            return page_->get_column_value<T>(column_index, row_index_);
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
        boost::optional<T> get_object(const std::string &column_name) const
        {
            auto column_index = resolve_index(column_name);
            return page_->get_column_value<T>(column_index, row_index_);
        }

        /**
         * Gets the row metadata.
         * @returns vector of column metadata, and returns empty vector if the result contains only an update count.
         *
         */
        const sql_row_metadata &row_metadata() const;

    private:
        std::size_t row_index_;
        const sql_page* page_;
        const sql_row_metadata *row_metadata_;

        std::size_t resolve_index(const std::string &column_name) const;
    };

    sql_page(std::vector<sql_column_type> column_types,
         std::vector<column> columns,
         bool last);

    sql_page(sql_page &&rhs) noexcept;

    sql_page(const sql_page &rhs) noexcept;

    sql_page &operator=(sql_page &&rhs) noexcept;

    sql_page &operator=(const sql_page &rhs) noexcept;

    const std::vector<sql_column_type>& column_types() const;

    bool last() const;

    std::size_t column_count() const;

    std::size_t row_count() const;

    const std::vector<sql_row>& rows() const;
private:
    friend class sql_result;

    std::vector<sql_column_type> column_types_;
    std::vector<column> columns_;
    std::vector<sql_row> rows_;
    bool last_;
    const sql_row_metadata *row_metadata_ = nullptr;

    template<typename T>
    boost::optional<T> get_column_value(std::size_t column_index, std::size_t row_index) const {
        assert(column_index < column_count());
        assert(row_index < row_count());

        auto &any_value = columns_[column_index][row_index];
        if (any_value.empty()) {
            return boost::none;
        }

        return boost::any_cast<T>(columns_[column_index][row_index]);
    }

    /**
     * set the row metadata on the page
     * @param row_meta the row metadata pointer
     */
    sql_page &row_metadata(const sql_row_metadata *row_meta);

    void construct_rows();
};

} // namespace sql
} // namespace client
} // namespace hazelcast
