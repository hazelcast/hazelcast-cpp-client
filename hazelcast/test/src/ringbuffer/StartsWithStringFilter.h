/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
            namespace rb {
                class StartsWithStringFilter {
                    friend serialization::hz_serializer<StartsWithStringFilter>;
                public:
                    StartsWithStringFilter(const std::string &start_string);
                private:
                    std::string start_string_;
                };
            }
        }
        namespace serialization {
            template<>
            struct hz_serializer<test::rb::StartsWithStringFilter> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::rb::StartsWithStringFilter &object, object_data_output &out);

                static test::rb::StartsWithStringFilter read_data(object_data_input &in);
            };
        }
    }
}


