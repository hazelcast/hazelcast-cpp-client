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
#include <memory>

#include "hazelcast/client/config/config_pattern_matcher.h"

namespace hazelcast {
namespace client {
namespace config {
namespace matcher {
/**
 * This {@code ConfigPatternMatcher} supports a simplified wildcard matching.
 * See "Config.md ## Using Wildcard" for details about the syntax options.
 * <p>
 * In addition the candidates are weighted by the best match. The best result is
 * returned. Throws {@link exception::ConfigurationException} is multiple
 * configurations are found.
 */
class HAZELCAST_API matching_point_config_pattern_matcher
  : public config_pattern_matcher
{
public:
    std::shared_ptr<std::string> matches(
      const std::vector<std::string>& config_patterns,
      const std::string& item_name) const override;

private:
    int get_matching_point(const std::string& pattern,
                           const std::string& item_name) const;
};
} // namespace matcher
} // namespace config
} // namespace client
} // namespace hazelcast
