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

#include <string>

#include <hazelcast/client/serialization/serialization.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                class StartsWithStringFilter {
                    friend serialization::hz_serializer<StartsWithStringFilter>;
                public:
                    StartsWithStringFilter(const std::string &startString);
                private:
                    std::string startString;
                };
            }
        }
        namespace serialization {
            template<>
            struct hz_serializer<test::ringbuffer::StartsWithStringFilter> : public identified_data_serializer {
                static int32_t getFactoryId();

                static int32_t getClassId();

                static void writeData(const test::ringbuffer::StartsWithStringFilter &object, ObjectDataOutput &out);

                static test::ringbuffer::StartsWithStringFilter readData(ObjectDataInput &in);
            };
        }
    }
}


