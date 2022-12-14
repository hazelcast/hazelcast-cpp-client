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
#include <boost/enable_shared_from_this.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_type.h"
#include "hazelcast/client/sql/sql_row_metadata.h"
#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
namespace protocol {
namespace codec {
namespace builtin {
class sql_page_codec;
}
} // namespace codec
} // namespace protocol
namespace sql {

/**
 * A finite set of rows returned to the client.
 */
class HAZELCAST_API sql_page
{
    using column = std::vector<boost::any>;
    struct HAZELCAST_API page_data;

public:
    class HAZELCAST_API sql_row
    {
    public:
        sql_row(size_t row_index, std::shared_ptr<page_data> page);

        /**
         * Gets the value of the column by index.
         * <p>
         * The class of the returned value depends on the SQL type of the
         * column. No implicit conversions are performed on the value.
         *
         * @param column_index column index, zero-based.
         * @return value of the column
         *
         * @throws hazelcast::client::exception::index_out_of_bounds if the
         * column index is out of bounds
         * @throws boost::bad_any_cast if the type of the column type isn't
         * assignable to the type \codeT\endcode
         *
         * @see row_metadata()
         * @see sql_column_metadata::type
         */
        template<typename T>
        boost::optional<T> get_object(std::size_t column_index) const
        {
            check_index(column_index);

            return page_data_->get_column_value<T>(column_index, row_index_);
        }

        /**
         * Gets the value of the column by column name.
         * <p>
         * Column name should be one of those defined in row_metadata,
         * case-sensitive. <p> The class of the returned value depends on the
         * SQL type of the column. No implicit conversions are performed on the
         * value.
         *
         * @param column_name column name
         * @return value of the column
         *
         * @throws hazelcast::client::exception::illegal_argument if a column
         * with the given name is not found
         * @throws boost::any_cast_exception if the type of the column type
         * isn't assignable to the type \codeT\endcode
         *
         * @see row_metadata()
         * @see sql_row_metadata::find_column
         * @see sql_column_metadata::name
         * @see sql_column_metadata::type
         */
        template<typename T>
        boost::optional<T> get_object(const std::string& column_name) const
        {
            auto column_index = resolve_index(column_name);
            return page_data_->get_column_value<T>(column_index, row_index_);
        }

        /**
         * Gets the row metadata.
         * @returns vector of column metadata, and returns empty vector if the
         * result contains only an update count.
         *
         * @see sql_row_metadata
         */
        const sql_row_metadata& row_metadata() const;

    private:
        std::size_t row_index_;
        std::shared_ptr<page_data> page_data_;

        std::size_t resolve_index(const std::string& column_name) const;

        void check_index(size_t index) const;
    };

    /**
     * Constructs an sql_page from the response returned from the server.
     * @param column_types The types of the columns in each row of the page.
     * @param columns The values of each column for all rows of the page.
     * @param last true if this is the last page in \sql_result, false
     * otherwise.
     * @param row_metadata The metadata of the rows of the page.
     */
    sql_page(std::vector<sql_column_type> column_types,
             std::vector<column> columns,
             bool last,
             std::shared_ptr<sql_row_metadata> row_metadata = nullptr);

    /**
     * Returns the types of the columns in each row.
     * @return the vector of column types in each row
     */
    const std::vector<sql_column_type>& column_types() const;

    /**
     *
     * @return true if this is the last page of the sql_result, false otherwise
     */
    bool last() const;

    /**
     * Returns the number of columns in each row.
     * @return the number of columns in each row.
     */
    std::size_t column_count() const;

    /**
     * Returns the number of rows in this page.
     * @return the number of rows in this page.
     */
    std::size_t row_count() const;

    /**
     * Returns the rows of this page.
     * @return the vector of rows in this page.
     */
    const std::vector<sql_row>& rows() const;

private:
    friend class sql_result;
    friend class protocol::codec::builtin::sql_page_codec;

    std::shared_ptr<page_data> page_data_;
    std::vector<sql_row> rows_;
    bool last_;

    /**
     * set the row metadata on the page
     * @param row_metadata the row metadata pointer
     */
    void row_metadata(std::shared_ptr<sql_row_metadata> row_metadata);

    /**
     * sets the serialization service to be used if column type is object
     * @param ss the serialization service pointer
     */
    void serialization_service(serialization::pimpl::SerializationService* ss);

    void construct_rows();

    struct HAZELCAST_API page_data
    {
        std::vector<sql_column_type> column_types_;
        std::vector<column> columns_;
        std::shared_ptr<sql_row_metadata> row_metadata_;
        serialization::pimpl::SerializationService* serialization_service_;

        template<typename T>
        boost::optional<T> get_column_value(std::size_t column_index,
                                            std::size_t row_index) const
        {
            assert(column_index < column_count());
            assert(row_index < row_count());

            auto& any_value = columns_[column_index][row_index];
            if (any_value.empty()) {
                return boost::none;
            }

            if (column_types_[column_index] != sql_column_type::object) {
                return boost::any_cast<T>(any_value);
            }

            // this is the object type, hence the value is `data`
            // and we need to de-serialize it
            return serialization_service_->to_object<T>(
              boost::any_cast<serialization::pimpl::data>(any_value));
        }

        std::size_t column_count() const;
        std::size_t row_count() const;
    };
};

} // namespace sql
} // namespace client
} // namespace hazelcast
