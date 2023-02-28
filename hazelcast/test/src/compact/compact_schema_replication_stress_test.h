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

#pragma once

#include <utility>

#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "compact_test_base.h"
#include "serialization/stress_type.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaReplicationStress : public compact_test_base
{};

template<int Current, int To>
struct schema_replicator;

template<int Current>
struct schema_replicator<Current, Current>
{
    schema_replicator(std::shared_ptr<imap> m, std::vector<schema>&) {}
};

template<int Current, int Until>
struct schema_replicator : schema_replicator<Current + 1, Until>
{
    schema_replicator(std::shared_ptr<imap> m, std::vector<schema>& schemas)
      : schema_replicator<Current + 1, Until>(m, schemas)
    {
        using namespace serialization::pimpl;

        stress_type<Current> instance{};
        auto schema = compact_stream_serializer::build_schema(instance);
        schemas.push_back(std::move(schema));
        m->put(Current, instance).get();
    }
};

template<int Current, int Until>
std::vector<serialization::pimpl::schema>
replicate_schemas(std::shared_ptr<imap> map)
{
    std::vector<serialization::pimpl::schema> schemas;
    schema_replicator<Current, Until>{ map, schemas };
    return schemas;
}

TEST_F(CompactSchemaReplicationStress, test)
{
    using replication_work_t =
      boost::future<std::vector<serialization::pimpl::schema>>;
    auto map = client.get_map(random_string()).get();

    replication_work_t replication_works[] = {
        boost::async(boost::launch::async, replicate_schemas<0, 10>, map),
        boost::async(boost::launch::async, replicate_schemas<10, 20>, map),
        boost::async(boost::launch::async, replicate_schemas<20, 30>, map),
        boost::async(boost::launch::async, replicate_schemas<30, 40>, map),
        boost::async(boost::launch::async, replicate_schemas<40, 50>, map),
        boost::async(boost::launch::async, replicate_schemas<50, 60>, map),
        boost::async(boost::launch::async, replicate_schemas<60, 70>, map),
        boost::async(boost::launch::async, replicate_schemas<70, 80>, map),
        boost::async(boost::launch::async, replicate_schemas<80, 90>, map),
        boost::async(boost::launch::async, replicate_schemas<90, 100>, map)
    };

    std::vector<serialization::pimpl::schema> replicated_schemas;

    auto append =
      [&replicated_schemas](std::vector<serialization::pimpl::schema> schemas) {
          replicated_schemas.insert(
            end(replicated_schemas), begin(schemas), end(schemas));
      };

    for (replication_work_t& work : replication_works)
        EXPECT_NO_THROW(append(work.get()));

    for (const serialization::pimpl::schema& s : replicated_schemas)
        EXPECT_TRUE(check_schema_on_backend(s));
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast