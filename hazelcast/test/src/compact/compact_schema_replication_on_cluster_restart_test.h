/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>
#include <sstream>
#include <functional>

#include <gtest/gtest.h>

#include "hazelcast/client/hazelcast_client.h"
#include "../HazelcastServerFactory.h"
#include "../HazelcastServer.h"
#include "../remote_controller_client.h"
#include "../TestHelperFunctions.h"

#include "compact_test_base.h"
#include "serialization/a_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaReplicationOnClusterRestart : public compact_test_base
{};

TEST_F(CompactSchemaReplicationOnClusterRestart, on_cluster_restart)
{
    auto schema_parent = get_schema<a_type>();
    auto schema_child = get_schema<nested_type>();

    auto map = client.get_map(random_string()).get();

    map->put(random_string(), a_type{}).get();

    member_.shutdown();

    HazelcastServer another_member{ factory_ };

    ASSERT_TRUE_EVENTUALLY(check_schema_on_backend(schema_parent) &&
                           check_schema_on_backend(schema_child));
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast