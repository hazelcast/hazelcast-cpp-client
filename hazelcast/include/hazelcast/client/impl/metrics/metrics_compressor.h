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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "hazelcast/client/impl/metrics/metric_descriptor.h"
#include "hazelcast/client/impl/metrics/metrics_dictionary.h"
#include "hazelcast/util/byte.h"
#include "hazelcast/util/export.h"
#include "hazelcast/util/Optional.h"

namespace hazelcast {
namespace client {
namespace impl {
namespace metrics {

class output_buffer
{
public:
    void write(byte val);
    void write(int32_t val);
    void write(int64_t val);
    void write(const std::string& str);
    void write(const std::vector<byte>& vec);
    const std::vector<byte>& content() const;
    std::vector<byte>& content();

private:
    std::vector<byte> buffer_;
};

class HAZELCAST_API metrics_compressor
{
public:
    void add_long(const metric_descriptor& descriptor, int64_t value);
    std::vector<byte> get_blob();

private:
    byte calculate_descriptor_mask(const metric_descriptor& descriptor);
    int get_dictionary_id(const util::optional<std::string>& word);
    void write_descriptor(const metric_descriptor& descriptor);
    void write_dictionary();

    int metrics_count{ 0 };
    metrics_dictionary dictionary_{};
    util::optional<metric_descriptor> last_descriptor_{};
    output_buffer metrics_buffer_{};
    output_buffer dictionary_buffer_{};
};

} // namespace metrics
} // namespace impl
} // namespace client
} // namespace hazelcast
