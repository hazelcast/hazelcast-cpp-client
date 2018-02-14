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
// Created by sancar koyunlu on 05/04/15.
//


#ifndef HAZELCAST_TestSerializationConstants
#define HAZELCAST_TestSerializationConstants


namespace hazelcast {
    namespace client {
        namespace test {
            namespace TestSerializationConstants {

                int const TEST_PORTABLE_FACTORY = 1;
                int const TEST_DATA_FACTORY = 1;

                int const CHILD_TEMPLATED_PORTABLE_1 = 1;
                int const CHILD_TEMPLATED_PORTABLE_2 = 2;
                int const OBJECT_CARRYING_PORTABLE = 4;
                int const PARENT_TEMPLATED_CONSTANTS = 5;
                int const TEST_DATA_SERIALIZABLE = 6;
                int const TEST_INNER_PORTABLE = 7;
                int const TEST_INVALID_READ_PORTABLE = 8;
                int const TEST_INVALID_WRITE_PORTABLE = 9;
                int const TEST_MAIN_PORTABLE = 10;
                int const TEST_NAMED_PORTABLE = 11;
                int const TEST_NAMED_PORTABLE_2 = 12;
                int const TEST_NAMED_PORTABLE_3 = 13;
                int const TEST_RAW_DATA_PORTABLE = 14;

                int const EMPLOYEE_FACTORY = 666;
                int const EMPLOYEE = 2;
            }
        }
    }
}

#endif //HAZELCAST_TestSerializationConstants
