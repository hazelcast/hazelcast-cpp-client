/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace partition {
                namespace strategy {

                    std::string StringPartitioningStrategy::getBaseName(const std::string &name) {
                        size_t indexOf = name.find('@');
                        if (indexOf == std::string::npos) {
                            return name;
                        }
                        return name.substr(0, indexOf);
                    }

                    std::string StringPartitioningStrategy::getPartitionKey(const std::string &key) {
                        size_t firstIndexOf = key.find('@');
                        if (firstIndexOf == std::string::npos) {
                            return key;
                        } else {
                            return key.substr(firstIndexOf + 1);
                        }
                    }
                }
            }
        }
    }
}

