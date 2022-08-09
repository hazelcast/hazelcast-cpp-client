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

#include <string>

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_type.h"

namespace hazelcast {
namespace client {
namespace sql {
namespace impl {

class HAZELCAST_API sql_page
{
    using column = std::vector<boost::any>;

public:
    sql_page(std::vector<sql_column_type> column_types,
         std::vector<column> columns,
         bool last)
      : column_types_(std::move(column_types))
      , columns_(std::move(columns))
      , last_(last)
    {}

    template<typename T>
    const boost::optional<T>& get_value(std::size_t c, std::size_t r) const;

    const std::vector<sql_column_type>& column_types() const
    {
        return column_types_;
    }

    const std::vector<column>& columns() const { return columns_; }

    bool last() const { return last_; }

private:
    std::vector<sql_column_type> column_types_;
    std::vector<column> columns_;
    bool last_;
};

template<>
inline const boost::optional<std::string>&
sql_page::get_value<std::string>(std::size_t c, std::size_t r) const
{
    return columns_[c][r];
}

} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
