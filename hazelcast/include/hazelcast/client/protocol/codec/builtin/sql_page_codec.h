/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <boost/optional.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/sql/sql_page.h"
#include "hazelcast/client/protocol/codec/builtin/list_cn_fixed_size_codec.h"

namespace hazelcast {
namespace client {
namespace protocol {
namespace codec {
namespace builtin {

class HAZELCAST_API sql_page_codec
{
public:
    static std::shared_ptr<sql::sql_page> decode(
      ClientMessage& msg,
      std::shared_ptr<sql::sql_row_metadata> row_metadata = nullptr);

private:
    template<typename T>
    static std::vector<boost::any> to_vector_of_any(
      std::vector<boost::optional<T>> values)
    {
        auto size = values.size();
        std::vector<boost::any> vector_of_any(size);
        for (std::size_t i = 0; i < size; ++i) {
            auto& value = values[i];
            if (value) {
                vector_of_any[i] = std::move(*value);
            }
        }
        return vector_of_any;
    }

    static std::vector<boost::any> decode_column_values(
      ClientMessage& msg,
      sql::sql_column_type column_type);
};

} // namespace builtin
} // namespace codec
} // namespace protocol
} // namespace client
} // namespace hazelcast
