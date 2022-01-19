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

#include <map>
#include <string>
#include <vector>

#include "hazelcast/util/byte.h"
#include "hazelcast/util/export.h"

namespace hazelcast {
namespace client {
namespace impl {
namespace metrics {

class HAZELCAST_API metrics_dictionary
{
private:
    using map = std::map<std::string, int>;

public:
    using const_iterator = map::const_iterator;

    int get_dictionary_id(const std::string& word);

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    std::size_t size() const noexcept;

private:
    map word_to_id;
};

} // namespace metrics
} // namespace impl
} // namespace client
} // namespace hazelcast
