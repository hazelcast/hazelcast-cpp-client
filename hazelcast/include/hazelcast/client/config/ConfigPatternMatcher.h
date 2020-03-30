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
#ifndef HAZELCAST_CLIENT_CONFIG_CONFIGPATTERNMATCHER_H_
#define HAZELCAST_CLIENT_CONFIG_CONFIGPATTERNMATCHER_H_

#include <string>
#include <vector>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * The ConfigPatternMatcher provides a strategy to match an item name to a configuration pattern.
             * <p>
             * It is used on each Config.getXXXConfig() and ClientConfig.getXXXConfig() call for map, list, queue, set, executor, topic,
             * semaphore etc., so for example {@code itemName} is the name of a map and <code>configPatterns</code> are all defined map
             * configurations.
             * <p>
             * If no configuration is found by the matcher it should return {@code NULL}. In this case the default config will be used
             * for this item then. If multiple configurations are found by the matcher a {@link exception::ConfigurationException}
             * should be thrown.
             * <p>
             * The default matcher is {@link hazelcast::config::matcher::MatchingPointConfigPatternMatcher}.
             */
            class HAZELCAST_API ConfigPatternMatcher {
            public:
                /**
                 * Returns the best match for an item name out of a list of configuration patterns.
                 *
                 * @param configPatterns list of configuration patterns
                 * @param itemName       item name to match
                 * @return a key of configPatterns which matches the item name or {@code NULL} if nothing matches
                 * @throws ConfigurationException if ambiguous configurations are found
                 */
                virtual std::shared_ptr<std::string>
                matches(const std::vector<std::string> &configPatterns, const std::string &itemName) const = 0;
            };
        }
    }
}


#endif /* HAZELCAST_CLIENT_CONFIG_CONFIGPATTERNMATCHER_H_ */
