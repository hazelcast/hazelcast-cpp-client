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

#include "hazelcast/client/config/config_pattern_matcher.h"
#include "hazelcast/util/SynchronizedMap.h"

namespace hazelcast {
namespace client {
namespace internal {
namespace config {
/**
 * Utility class to access configuration.
 */
class ConfigUtils
{
public:
    template<typename T>
    static const T* lookup_by_pattern(
      const client::config::config_pattern_matcher& config_pattern_matcher,
      const std::unordered_map<std::string, T>& config_patterns,
      const std::string& item_name)
    {
        auto candidate = config_patterns.find(item_name);
        if (candidate != config_patterns.end()) {
            return &candidate->second;
        }
        auto size = config_patterns.size();
        std::vector<std::string> keys(size);
        size_t index = 0;
        for (const auto& e : config_patterns) {
            keys[index] = e.first;
        }
        std::shared_ptr<std::string> configPatternKey =
          config_pattern_matcher.matches(keys, item_name);
        if (configPatternKey) {
            candidate = config_patterns.find(*configPatternKey);
            if (candidate != config_patterns.end()) {
                return &candidate->second;
            }
        }
        return nullptr;
    }
};
} // namespace config
} // namespace internal
} // namespace client
}; // namespace hazelcast
