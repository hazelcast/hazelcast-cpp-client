/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/config/ConfigPatternMatcher.h"
#include "hazelcast/util/SynchronizedMap.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace config {
                /**
                 * Utility class to access configuration.
                 */
                class ConfigUtils {
                public:
                    template<typename T>
                    static const T *
                    lookup_by_pattern(const client::config::ConfigPatternMatcher &configPatternMatcher,
                                    const std::unordered_map<std::string, T> &configPatterns, const std::string &itemName) {
                        auto candidate = configPatterns.find(itemName);
                        if (candidate != configPatterns.end()) {
                            return &candidate->second;
                        }
                        auto size = configPatterns.size();
                        std::vector<std::string> keys(size);
                        size_t index = 0;
                        for (const auto &e : configPatterns) {
                            keys[index] = e.first;
                        }
                        std::shared_ptr<std::string> configPatternKey = configPatternMatcher.matches(
                                keys, itemName);
                        if (configPatternKey) {
                            candidate = configPatterns.find(*configPatternKey);
                            if (candidate != configPatterns.end()) {
                                return &candidate->second;
                            }
                        }
                        return nullptr;
                    }
                };
            }
        }
    }
};


