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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <hazelcast/client/query/Predicates.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    auto map = hz.getMap("map");

    map->put("item1", hazelcast::client::HazelcastJsonValue("{ \"age\": 4 }")).get();
    map->put("item2", hazelcast::client::HazelcastJsonValue("{ \"age\": 20 }")).get();

    // Get the objects whose age is less than 6
    auto result = map->values<hazelcast::client::HazelcastJsonValue>(
            hazelcast::client::query::GreaterLessPredicate(hz, "age", 6, false, true)).get();

    std::cout << "Retrieved " << result.size() << " values whose age is less than 6." << std::endl;
    std::cout << "Entry is:" << result[0].toString() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
