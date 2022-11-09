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

#include <string>

#include "hazelcast/util/Optional.h"
#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace impl {
namespace metrics {

enum struct probe_unit
{
    BYTES = 0,
    MS = 1,
    NS = 2,
    PERCENT = 3,
    COUNT = 4,
    BOOLEAN = 5,
    ENUM = 6,

    NONE = -1
};

class HAZELCAST_API metric_descriptor
{
public:
    metric_descriptor(std::string prefix,
                      std::string metric,
                      std::string discriminator,
                      std::string discriminator_value,
                      probe_unit unit = probe_unit::NONE);

    metric_descriptor(std::string prefix,
                      std::string metric,
                      probe_unit unit = probe_unit::NONE);

    const std::string& prefix() const;
    const std::string& metric() const;
    const util::optional<std::string>& discriminator() const;
    const util::optional<std::string>& discriminator_value() const;
    probe_unit unit() const;

private:
    std::string prefix_;
    std::string metric_;
    util::optional<std::string> discriminator_;
    util::optional<std::string> discriminator_value_;
    probe_unit unit_;
};

} // namespace metrics
} // namespace impl
} // namespace client
} // namespace hazelcast
