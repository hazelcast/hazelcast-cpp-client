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
// Created by ihsan demir on 14 Apr 2016.

#ifndef HAZELCAST_UTIL_SORTINGUTIL_
#define HAZELCAST_UTIL_SORTINGUTIL_

#include <utility>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class SortingUtil {
        public:
            /**
             * comparison function object returns ​true if the first argument is less than (i.e. is ordered before) the second.
             *
             * @param lhs First number to compare with second one
             * @param rhs Second number to compare with first one
             * @return true if lhs < rhs, false otherwise. If lhs is NULL return true, if rhs is NULL return false;
             */
            template <typename T>
            static bool less(const T *lhs, const T *rhs) {
                if (NULL == lhs) {
                    return true;
                }
                if (NULL == rhs) {
                    return false;
                }
                return *lhs < *rhs;
            }
        };
    }
}


#endif //HAZELCAST_UTIL_SORTINGUTIL_

