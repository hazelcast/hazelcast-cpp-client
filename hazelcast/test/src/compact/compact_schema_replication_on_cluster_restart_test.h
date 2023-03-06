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
#include "serialization/a_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaReplicationOnClusterRestart : public compact_test_base
{
protected:
    int number_of_schemas_on_member()
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          R"(
              var length = instance_0.getOriginal().node.getSchemaService().getAllSchemas().size();
              result = "" + length;
          )",
          Lang::JAVASCRIPT);

        if (response.result.empty()) {
            (void)[]
            {
                GTEST_FAIL();
            }
            ();
        }

        return std::stoi(response.result);
    }
};

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

TEST_F(CompactSchemaReplicationOnClusterRestart,
       dont_replicate_if_there_are_no_schema_on_cluster_restart)
{
    auto condition = [this]() { return number_of_schemas_on_member() == 0; };

    ASSERT_TRUE_EVENTUALLY(condition());

    member_.shutdown();

    HazelcastServer another_member{ factory_ };

    ASSERT_TRUE_ALL_THE_TIME(condition(), 1);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast