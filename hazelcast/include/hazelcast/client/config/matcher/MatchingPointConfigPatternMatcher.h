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
#include <memory>

#include "hazelcast/client/config/ConfigPatternMatcher.h"

namespace hazelcast {
    namespace client {
        namespace config {
            namespace matcher {
                /**
                 * This {@code ConfigPatternMatcher} supports a simplified wildcard matching.
                 * See "Config.md ## Using Wildcard" for details about the syntax options.
                 * <p>
                 * In addition the candidates are weighted by the best match. The best result is returned.
                 * Throws {@link exception::ConfigurationException} is multiple configurations are found.
                 */
                class HAZELCAST_API MatchingPointConfigPatternMatcher : public ConfigPatternMatcher {
                public:
                    virtual std::shared_ptr<std::string>
                    matches(const std::vector<std::string> &configPatterns, const std::string &itemName) const;

                private:
                    int getMatchingPoint(const std::string &pattern, const std::string &itemName) const;
                };
            }
        }
    }
}


