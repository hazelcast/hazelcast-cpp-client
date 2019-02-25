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
//
// Created by sancar koyunlu on 15/05/14.
//

#ifndef HAZELCAST_TestHelperFunctions
#define HAZELCAST_TestHelperFunctions

#define ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT_MSG(message, expected, actual, timeoutSeconds) do{              \
            bool __result__ = false;                                \
            for(int i = 0 ; i < timeoutSeconds * 5 && !__result__ ; i++ ) {    \
                if ((expected) == (actual)) {                       \
                    __result__ = true;                              \
                } else {                                        \
                    util::sleepmillis(200);                     \
                }                                               \
            }                                                   \
            ASSERT_TRUE(__result__) << message;                     \
      }while(0)                                                 \

#define WAIT_TRUE_EVENTUALLY(expression) do{                    \
            for(int i = 0 ; i < 5 * 120 && !(expression) ; i++ ) { \
                util::sleepmillis(200);                         \
            }                                                   \
      }while(0)                                                 \

#define ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT(expected, actual, timeoutSeconds) ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT_MSG("", expected, actual, timeoutSeconds)
#define ASSERT_EQ_EVENTUALLY(expected, actual) ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT(expected, actual, 120)

#define ASSERT_TRUE_ALL_THE_TIME(expression, seconds) do{       \
            for(int i = 0; i < seconds ; i++ ) {                \
                ASSERT_TRUE(expression);                        \
                util::sleepmillis(1000);                        \
            }                                                   \
      } while(0)                                                \

#define WAIT_EQ_EVENTUALLY(expected, expression) WAIT_TRUE_EVENTUALLY((expected) == (expression))
#define WAIT_NE_EVENTUALLY(expected, expression) WAIT_NE_EVENTUALLY((expected) != (expression))

#define ASSERT_EQ_PTR(expected, actual, type) do{ \
ASSERT_NE((type *) NULL, actual);                   \
ASSERT_EQ((expected), (*actual));                 \
} while(0) \

#define ASSERT_NULL(msg, value, type) ASSERT_EQ((type *) NULL, value) << msg
#define ASSERT_NOTNULL(value, type) ASSERT_NE((type *) NULL, value)
#define ASSERT_TRUE_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, true)
#define ASSERT_TRUE_EVENTUALLY_WITH_TIMEOUT(value, timeout) ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT(value, true, timeout)
#define ASSERT_NULL_EVENTUALLY(value, type) ASSERT_EQ_EVENTUALLY((type *) NULL, value)
#define ASSERT_OPEN_EVENTUALLY(latch) ASSERT_TRUE((latch).await(120))

#define assertSizeEventually(expectedSize, container) assertSizeEventuallyWithTimeout(expectedSize, container, 120)
#define assertSizeEventuallyWithTimeout(expectedSize, container, timeoutSeconds) do{  \
    ASSERT_EQ_EVENTUALLY_WITH_TIMEOUT_MSG("the size of the map is not correct", expectedSize, (container).size(), timeoutSeconds);  \
    } while(0)                                                                                                                      \

#endif //HAZELCAST_TestHelperFunctions
