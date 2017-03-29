/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_UTIL_COMPARATOR_
#define HAZELCAST_UTIL_COMPARATOR_

namespace hazelcast {
    namespace util {
        template <typename T>
        class Comparator {
        public:
            virtual ~Comparator() { }

            /**
             * @lhs First value to compare
             * @rhs Second value to compare
             * @return Returns < 0 if lhs is less, >0 if lhs is greater, else returns 0.
             */
            virtual int compare(const T *lhs, const T *rhs) const = 0;
        };
    }
}


#endif //HAZELCAST_UTIL_COMPARATOR_

