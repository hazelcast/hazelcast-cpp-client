/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_CONFIG_CONFIGUTILS_H_
#define HAZELCAST_CLIENT_INTERNAL_CONFIG_CONFIGUTILS_H_

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
                    static boost::shared_ptr<T>
                    lookupByPattern(const client::config::ConfigPatternMatcher &configPatternMatcher,
                                    util::SynchronizedMap<std::string, T> &configPatterns, const std::string &itemName) {
                        boost::shared_ptr<T> candidate = configPatterns.get(itemName);
                        if (candidate.get() != NULL) {
                            return candidate;
                        }
                        boost::shared_ptr<std::string> configPatternKey = configPatternMatcher.matches(
                                configPatterns.keys(), itemName);
                        if (configPatternKey.get() != NULL) {
                            return configPatterns.get(*configPatternKey);
                        }
                        return boost::shared_ptr<T>();
                    }
                };
            }
        }
    }
};

#endif /* HAZELCAST_CLIENT_INTERNAL_CONFIG_CONFIGUTILS_H_ */
