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
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/config/ClientFlakeIdGeneratorConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {

            ClientFlakeIdGeneratorConfig::ClientFlakeIdGeneratorConfig(const std::string &name)
                    : name(name), prefetchCount(DEFAULT_PREFETCH_COUNT),
                      prefetchValidityMillis(DEFAULT_PREFETCH_VALIDITY_MILLIS) {}

            const std::string &ClientFlakeIdGeneratorConfig::getName() const {
                return name;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setName(const std::string &name) {
                ClientFlakeIdGeneratorConfig::name = name;
                return *this;
            }

            int32_t ClientFlakeIdGeneratorConfig::getPrefetchCount() const {
                return prefetchCount;
            }

            ClientFlakeIdGeneratorConfig &ClientFlakeIdGeneratorConfig::setPrefetchCount(int32_t prefetchCount) {
                std::ostringstream out;
                out << "prefetch-count must be 1.." << MAXIMUM_PREFETCH_COUNT << ", not " << prefetchCount;
                util::Preconditions::checkTrue(prefetchCount > 0 && prefetchCount <= MAXIMUM_PREFETCH_COUNT, out.str());
                ClientFlakeIdGeneratorConfig::prefetchCount = prefetchCount;
                return *this;
            }

            int64_t ClientFlakeIdGeneratorConfig::getPrefetchValidityMillis() const {
                return prefetchValidityMillis;
            }

            ClientFlakeIdGeneratorConfig &
            ClientFlakeIdGeneratorConfig::setPrefetchValidityMillis(int64_t prefetchValidityMillis) {
                util::Preconditions::checkNotNegative(prefetchValidityMillis,
                                                      "prefetchValidityMs must be non negative");
                ClientFlakeIdGeneratorConfig::prefetchValidityMillis = prefetchValidityMillis;
                return *this;
            }
        }
    }
}

