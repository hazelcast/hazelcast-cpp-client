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
#include <unordered_map>

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_metadata.h"

namespace hazelcast {
namespace client {
namespace sql {

/**
 * SQL row metadata.
 */
class HAZELCAST_API sql_row_metadata
{
public:
    /**
     * key is the column name, value is the column index.
     */
    using const_iterator =
      std::unordered_map<std::string, std::size_t>::const_iterator;

    explicit sql_row_metadata(std::vector<sql_column_metadata> columns);

    /**
     * Gets the number of columns in the row.
     *
     * @return the number of columns in the row
     */
    std::size_t column_count() const;

    /**
     * Gets column metadata.
     *
     * @param index column index, zero-based
     * @return column metadata
     * @throws IndexOutOfBoundsException If the column index is out of bounds
     */
    const sql_column_metadata& column(std::size_t index) const;

    /**
     * Gets columns metadata.
     *
     * @return columns metadata
     */
    const std::vector<sql_column_metadata>& columns() const;

    /**
     * Find index of the column with the given name. Returned index can be used
     * to get column value from sql_row.
     *
     * @param column_name column name (case sensitive)
     * @return the iterator pointing to the found item or end() if not found.
     *
     * @see sql_row
     */
    const_iterator find_column(const std::string& column_name) const;

    /**
     * Constant indicating that the column is not found.
     */
    const_iterator end() const;

    friend bool HAZELCAST_API operator==(const sql_row_metadata& lhs,
                           const sql_row_metadata& rhs);

private:
    std::vector<sql_column_metadata> columns_;
    std::unordered_map<std::string, std::size_t> name_to_index_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
