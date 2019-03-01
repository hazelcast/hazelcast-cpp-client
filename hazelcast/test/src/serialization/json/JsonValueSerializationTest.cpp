/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "ClientTestSupport.h"

#include <hazelcast/client/serialization/json/HazelcastJsonValue.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class JsonValueSerializationTest : public ClientTestSupport {
            public:
                JsonValueSerializationTest() : serializationService(SerializationConfig()) {}

            protected:
                serialization::pimpl::SerializationService serializationService;
            };

            TEST_F(JsonValueSerializationTest, testSerializeDeserializeJsonValue) {
                serialization::json::HazelcastJsonValue jsonValue("{ \"key\": \"value\" }");
                serialization::pimpl::Data jsonData = serializationService.toData(&jsonValue);
                std::auto_ptr<serialization::json::HazelcastJsonValue> jsonDeserialized(
                        serializationService.toObject<serialization::json::HazelcastJsonValue>(jsonData));
                ASSERT_EQ_PTR(jsonValue, jsonDeserialized.get(), serialization::json::HazelcastJsonValue);
            }
        }
    }
}
