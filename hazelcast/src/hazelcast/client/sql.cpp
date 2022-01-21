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

#include "hazelcast/client/sql/impl/query_id.h"

namespace hazelcast {
namespace client {
namespace sql {
namespace impl {

query_id::query_id(int64_t member_id_high,
                           int64_t member_id_low,
                           int64_t local_id_high,
                           int64_t local_id_low)
  : member_id_high_{ member_id_high }
  , member_id_low_{ member_id_low }
  , local_id_high_{ local_id_high }
  , local_id_low_{ local_id_low }
{}

int64_t
query_id::member_id_high() const
{
    return member_id_high_;
}

int64_t
query_id::member_id_low() const
{
    return member_id_low_;
}

int64_t
query_id::local_id_high() const
{
    return local_id_high_;
}

int64_t
query_id::local_id_low() const
{
    return local_id_low_;
}

} // namespace impl
} // namespace sql
} // namespace client
} // namespace hazelcast
