/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include "serialization/sample_compact_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaReplicationOnWrite : public compact_test_base
{};

TEST_F(CompactSchemaReplicationOnWrite, imap_put)
{
    auto schema_parent = get_schema<sample_compact_type>();
    auto schema_child = get_schema<nested_type>();

    ASSERT_EQ(check_schema_on_backend(schema_parent), false);
    ASSERT_EQ(check_schema_on_backend(schema_child), false);

    auto map = client->get_map(random_string()).get();

    map->put(random_string(), sample_compact_type{}).get();

    ASSERT_EQ(check_schema_on_backend(schema_parent), true);
    ASSERT_EQ(check_schema_on_backend(schema_child), true);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast