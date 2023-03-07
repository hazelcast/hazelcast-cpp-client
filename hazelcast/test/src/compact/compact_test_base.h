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

#include <gtest/gtest.h>

#include "hazelcast/client/hazelcast_client.h"
#include "../HazelcastServerFactory.h"
#include "../HazelcastServer.h"
#include "../remote_controller_client.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class compact_test_base : public testing::Test
{
public:
    using schema_t = serialization::pimpl::schema;

    compact_test_base()
      : factory_{ "hazelcast/test/resources/compact.xml" }
      , member_{ factory_ }
      , client{ new_client(config()).get() }
    {
        remote_controller_client().ping();
    }

protected:
    void SetUp() override
    {
        auto version = client.get_cluster().get_members().front().get_version();

        if (version < member::version{ 5, 2, 0 })
            GTEST_SKIP();
    }

    template<typename T>
    void replicate_schema()
    {
        auto schema = get_schema<T>();

        spi::ClientContext context{ client };

        ASSERT_NO_THROW(
          context.get_schema_service().replicate_schema_in_cluster(schema));
    }

    bool check_schema_on_backend(const schema_t& schema)
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                        var schemas = instance_0.getOriginal().node.getSchemaService().getAllSchemas();
                        var iterator = schemas.iterator();

                        var exist = false;
                        while(iterator.hasNext()){
                            var schema = iterator.next();

                            if (schema.getSchemaId() == "%1%")
                                exist = true;
                        }

                        result = "" + exist;
                    )") %
           schema.schema_id())
            .str(),
          Lang::JAVASCRIPT);

        return response.result == "true";
    }

    template<typename T>
    schema_t get_schema()
    {
        using namespace serialization::pimpl;

        return compact_stream_serializer::build_schema(T{});
    }

    HazelcastServerFactory factory_;
    HazelcastServer member_;
    hazelcast_client client;

private:
    static client_config config()
    {
        client_config cfg;

        cfg.set_cluster_name("compact-dev");
        cfg.get_logger_config().level(logger::level::finest);

        return cfg;
    }
};

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast