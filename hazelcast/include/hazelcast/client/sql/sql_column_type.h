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

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace sql {

enum class HAZELCAST_API sql_column_type
{
    /** VARCHAR type, represented by std::string */
    varchar = 0,

    /** BOOLEAN type, represented by bool */
    boolean = 1,

    /** TINYINT type, represented by byte */
    tinyint = 2,

    /** SMALLINT type, represented by int16_t */
    smallint = 3,

    /** INTEGER type, represented by int32_t */
    integer = 4,

    /** BIGINT type, represented by int64_t */
    bigint = 5,

    /** DECIMAL type, represented by hazelcast::client::big_decimal */
    decimal = 6,

    /** REAL type, represented by float */
    real = 7,

    /** DOUBLE type, represented by double */
    double_ = 8,

    /** DATE type, represented by hazelcast::client::local_date */
    date = 9,

    /** TIME type, represented by hazelcast::client::local_time */
    time = 10,

    /** TIMESTAMP type, represented by hazelcast::client::local_date_time */
    timestamp = 11,

    /** TIMESTAMP_WITH_TIME_ZONE type, represented by hazelcast::client::offset_date_time */
    timestamp_with_timezone = 12,

    /** OBJECT type, could be represented by any Hazelcast serializable class. */
    object = 13,

    /**
     * The type of the generic SQL \c NULL literal.
     * <p>
     * The only valid value of \c NULL type is \code{.cpp}nullptr_t\endcode.
     */
    null = 14
};

} // namespace sql
} // namespace client
} // namespace hazelcast
