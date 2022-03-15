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

namespace hazelcast {
namespace client {
namespace sql {

enum class column_type
{
    varchar = 0,
    boolean = 1,
    tinyint = 2,
    smallint = 3,
    integer = 4,
    bigint = 5,
    decimal = 6,
    real = 7,
    double_ = 8,
    date = 9,
    time = 10,
    timestamp = 11,
    timestamp_with_timezone = 12,
    object = 13,
    null = 14
};

} // namespace sql
} // namespace client
} // namespace hazelcast
