/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include "hazelcast/client/sql/sql_column_metadata.h"

namespace hazelcast {
namespace client {
namespace protocol {
namespace codec {
namespace builtin {
class HAZELCAST_API custom_type_factory
{
public:
    static sql::sql_column_metadata create_sql_column_metadata(
      std::string name,
      int32_t type,
      bool is_nullable_exists,
      bool nullability);
};

} // namespace builtin
} // namespace codec
} // namespace protocol
} // namespace client
} // namespace hazelcast
