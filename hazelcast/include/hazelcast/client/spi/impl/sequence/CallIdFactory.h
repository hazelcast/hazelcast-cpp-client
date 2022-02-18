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

#include <stdint.h>
#include <memory>

#include "hazelcast/util/export.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace spi {
namespace impl {
namespace sequence {
class HAZELCAST_API CallIdFactory
{
public:
    static std::unique_ptr<CallIdSequence> new_call_id_sequence(
      bool is_back_pressure_enabled,
      int32_t max_allowed_concurrent_invocations,
      int64_t backoff_timeout_ms);
};
} // namespace sequence
} // namespace impl
} // namespace spi
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
