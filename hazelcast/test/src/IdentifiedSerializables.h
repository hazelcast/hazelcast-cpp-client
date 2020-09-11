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

#include <hazelcast/client/serialization/serialization.h>

namespace hazelcast {
    namespace client {
        namespace test {
            enum struct identified_class_ids {
                MULTIPLICATION = 16,
                APPEND_STRING = 17
            };

            struct multiplication {
                std::int64_t multiplier;
            };

            struct append_string {
                std::string suffix;
            };
        }

        namespace serialization {
            struct identified_base : public identified_data_serializer {
                static int32_t getFactoryId() {
                    return 66;
                }
            };

            template<>
            struct hz_serializer<test::multiplication> : public identified_base {

                static int32_t getClassId() {
                    return static_cast<int32_t>(test::identified_class_ids::MULTIPLICATION);
                }

                static void writeData(const test::multiplication &object, ObjectDataOutput &out) {
                    out.write(object.multiplier);
                }

                static struct test::multiplication readData(ObjectDataInput &in) {
                    return {in.read<int64_t>()};
                }
            };

            template<>
            struct hz_serializer<test::append_string> : public identified_base {

                static int32_t getClassId() {
                    return static_cast<int32_t>(test::identified_class_ids::APPEND_STRING);
                }

                static void writeData(const test::append_string &object, ObjectDataOutput &out) {
                    out.write(object.suffix);
                }

                static struct test::append_string readData(ObjectDataInput &in) {
                    return {in.read<std::string>()};
                }
            };
        }
    }
}
