/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 22 Nov 2016.

#include "hazelcast/client/config/NearCachePreloaderConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            bool NearCachePreloaderConfig::isEnabled() const {
                return enabled;
            }

            std::ostream &operator<<(std::ostream &out, const NearCachePreloaderConfig &config) {
                //TODO
                return out;
            }
        }
    }
}

