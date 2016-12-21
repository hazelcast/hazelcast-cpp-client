/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 9 Dec 2016.

#include <cstdlib>
#include <climits>

#include "hazelcast/util/HashUtil.h"
#include "hazelcast/util/Preconditions.h"

namespace hazelcast {
    namespace util {
        int HashUtil::hashToIndex(int hash, int length) {
            Preconditions::checkPositive(length, "mod must be larger than 0");

            if (hash == INT_MIN) {
                hash = 0;
            } else {
                hash = std::abs(hash);
            }

            return hash % length;
        }
    }
}


