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
namespace hazelcast {
    namespace client {
        namespace test {
            class IdentifiedFactory {
            public:
                static const int FACTORY_ID = 66;

                enum CLASS_IDS {
                    APPEND_CALLABLE = 5,
                    CANCELLATION_AWARE_TASK = 6,
                    FAILING_CALLABLE = 7,
                    GET_MEMBER_UUID_TASK = 8,
                    MAP_PUTPARTITIONAWARE_CALLABLE = 9,
                    NULL_CALLABLE = 10,
                    SELECT_ALL_MEMBERS = 11,
                    SELECT_NO_MEMBERS = 12,
                    SERIALIZED_COUNTER_CALLABLE = 13,
                    TASK_WITH_UNSERIALIZABLE_RESPONSE = 14
                };
            };
        }
    }
}

