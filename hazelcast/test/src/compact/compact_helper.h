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

#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

inline hazelcast::client::local_time
current_time()
{
    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    return hazelcast::client::local_time{ static_cast<uint8_t>(now->tm_hour),
                                          static_cast<uint8_t>(now->tm_min),
                                          static_cast<uint8_t>(now->tm_sec),
                                          0 };
}

inline hazelcast::client::local_date
current_date()
{
    std::time_t t = std::time(nullptr); // get time now
    std::tm* now = std::localtime(&t);
    return hazelcast::client::local_date{ now->tm_year + 1900,
                                          static_cast<uint8_t>(now->tm_mon),
                                          static_cast<uint8_t>(now->tm_mday) };
}

inline hazelcast::client::local_date_time
current_timestamp()
{
    return hazelcast::client::local_date_time{ current_date(), current_time() };
}

inline hazelcast::client::offset_date_time
current_timestamp_with_timezone()
{
    return hazelcast::client::offset_date_time{ current_timestamp(), 3600 };
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast
