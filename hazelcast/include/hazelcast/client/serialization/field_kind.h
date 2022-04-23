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

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
enum HAZELCAST_API field_kind
{
    BOOLEAN = 0,
    ARRAY_OF_BOOLEAN = 1,
    INT8 = 2,
    ARRAY_OF_INT8 = 3,
    INT16 = 6,
    ARRAY_OF_INT16 = 7,
    INT32 = 8,
    ARRAY_OF_INT32 = 9,
    INT64 = 10,
    ARRAY_OF_INT64 = 11,
    FLOAT32 = 12,
    ARRAY_OF_FLOAT32 = 13,
    FLOAT64 = 14,
    ARRAY_OF_FLOAT64 = 15,
    STRING = 16,
    ARRAY_OF_STRING = 17,
    DECIMAL = 18,
    ARRAY_OF_DECIMAL = 19,
    TIME = 20,
    ARRAY_OF_TIME = 21,
    DATE = 22,
    ARRAY_OF_DATE = 23,
    TIMESTAMP_WITH_TIMEZONE = 26,
    ARRAY_OF_TIMESTAMP_WITH_TIMEZONE = 27,
    COMPACT = 28,
    ARRAY_OF_COMPACT = 29,
    NULLABLE_BOOLEAN = 32,
    ARRAY_OF_NULLABLE_BOOLEAN = 33,
    NULLABLE_INT8 = 34,
    ARRAY_OF_NULLABLE_INT8 = 35,
    NULLABLE_INT16 = 36,
    ARRAY_OF_NULLABLE_INT16 = 37,
    NULLABLE_INT32 = 38,
    ARRAY_OF_NULLABLE_INT32 = 39,
    NULLABLE_INT64 = 40,
    ARRAY_OF_NULLABLE_INT64 = 41,
    NULLABLE_FLOAT32 = 42,
    ARRAY_OF_NULLABLE_FLOAT32 = 43,
    NULLABLE_FLOAT64 = 44,
    ARRAY_OF_NULLABLE_FLOAT64 = 45,
};
static const int NUMBER_OF_FIELD_KINDS = ARRAY_OF_NULLABLE_FLOAT64 + 1;
} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
