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

#include "hazelcast/util/HazelcastDll.h"
namespace hazelcast {
    namespace util {
        class HAZELCAST_API HashUtil {
        public:
            /**
             * A function that calculates the index (e.g. to be used in an array/list) for a given hash. The returned value will always
             * be equal or larger than 0 and will always be smaller than 'length'.
             *
             * The reason this function exists is to deal correctly with negative and especially the Integer.MIN_VALUE; since that can't
             * be used safely with a Math.abs function.
             *
             * @param length the length of the array/list
             * @return the mod of the hash
             * @throws IllegalArgumentException if mod smaller than 1.
             */
            static int hashToIndex(int hash, int length);
        };
    }
}




