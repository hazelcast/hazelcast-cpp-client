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
// Created by sancar koyunlu on 15/05/14.
//

#ifndef HAZELCAST_TestHelperFunctions
#define HAZELCAST_TestHelperFunctions

#define ASSERT_EQ_EVENTUALLY(expected, actual) do{              \
            bool result = false;                                \
            for(int i = 0 ; i < 60 && !result ; i++ ) {         \
                if (expected == actual) {                       \
                    result = true;                              \
                } else {                                        \
                    util::sleep(2);                             \
                }                                               \
            }                                                   \
            ASSERT_TRUE(result);                                \
      }while(0)                                                 \

#define ASSERT_NE_EVENTUALLY(expected, actual) do{              \
            bool result = false;                                \
            for(int i = 0 ; i < 60 && !result ; i++ ) {         \
                if (expected != actual) {                       \
                    result = true;                              \
                } else {                                        \
                    util::sleep(2);                             \
                }                                               \
            }                                                   \
            ASSERT_TRUE(result);                                \
      }while(0)                                                 \

#define ASSERT_TRUE_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, true)
#define ASSERT_FALSE_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, false)
#define ASSERT_NULL_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, NULL)
#define ASSERT_NOTNULL_EVENTUALLY(value) ASSERT_NE_EVENTUALLY(value, NULL)

#endif //HAZELCAST_TestHelperFunctions
