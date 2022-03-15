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

class HAZELCAST_API row
{
public:
    template<typename T>
    const boost::optional<T>& get_value(std::size_t column) const
    {
        return page_.template get_value<T>(column, id_);
    }

    std::size_t id_;
    impl::page& page_;
};

// TODO multi-page support
class HAZELCAST_API result
{
public:
    result(int64_t update_count,
           boost::optional<std::vector<column_metadata>> row_metadata,
           boost::optional<impl::page> first_page);

    int64_t update_count() const;

    bool is_row_set() const;

    boost::future<std::vector<row>> fetch_page();

    bool has_more() const;

    const boost::optional<std::vector<column_metadata>>& row_metadata() const;

private:
    int64_t update_count_;
    boost::optional<std::vector<column_metadata>> row_metadata_;
    boost::optional<impl::page> current_page_;
};

} // namespace sql
} // namespace client
} // namespace hazelcast
