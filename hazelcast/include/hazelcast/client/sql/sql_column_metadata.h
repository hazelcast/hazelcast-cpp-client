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

struct HAZELCAST_API sql_column_metadata
{
    std::string name;
    sql_column_type type;
    bool nullable;

    friend bool HAZELCAST_API operator==(const sql_column_metadata& lhs,
                    const sql_column_metadata& rhs);
};

} // namespace sql
} // namespace client
} // namespace hazelcast
