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

#include <sstream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "compact_test_base.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactSchemaValidation : public compact_test_base
{
protected:
    using schema_t = hazelcast::client::serialization::pimpl::schema;
    using field_descriptor_t =
      hazelcast::client::serialization::pimpl::field_descriptor;

    struct schema_proxy
    {
        std::string type_name;
        std::vector<field_descriptor_t> descriptors;
    };

    schema_proxy read_schema_from_server(const schema_t& schema)
    {
        Response response;

        remote_controller_client().executeOnController(
          response,
          factory_.get_cluster_id(),
          (boost::format(
             R"(
                        var schemas = instance_0.getOriginal().node.getSchemaService().getAllSchemas();
                        var iterator = schemas.iterator();

                        var schema;
                        while(iterator.hasNext()){
                            var schema = iterator.next();

                            if (schema.getSchemaId() == "%1%")
                                break;
                        }

                        var schema_obj = {};

                        schema_obj.type_name = schema.getTypeName();
                        schema_obj.fields = [];
                        
                        iterator = schema.getFields().iterator();

                        while(iterator.hasNext()){
                            var descriptor = iterator.next();

                            schema_obj.fields.push(
                                {
                                    kind: descriptor.getKind().getId(),
                                    field_name: descriptor.getFieldName(),
                                    index: descriptor.getIndex(),
                                    offset: descriptor.getOffset(),
                                    bitOffset: descriptor.getBitOffset()
                                }
                            );
                        }

                        result = "" + JSON.stringify(schema_obj);
                    )") %
           schema.schema_id())
            .str(),
          Lang::JAVASCRIPT);

        return json_to_proxy(response.result);
    }

private:
    schema_proxy json_to_proxy(const std::string& text)
    {
        using namespace boost::property_tree;

        std::stringstream input;

        input << text;

        ptree root;
        read_json(input, root);

        schema_proxy proxy;
        proxy.type_name = root.get<std::string>("type_name");

        for (ptree::value_type& field : root.get_child("fields")) {
            proxy.descriptors.push_back(
              { serialization::field_kind(field.second.get<int>("kind")),
                field.second.get<int>("index"),
                field.second.get<int>("offset"),
                field.second.get<int8_t>("bitOffset") });
        }

        return proxy;
    }
};

TEST_F(CompactSchemaValidation, validate)
{
    auto schema = get_schema<sample_compact_type>();

    replicate_schema<sample_compact_type>();

    auto actual = read_schema_from_server(schema);

    EXPECT_EQ(schema.type_name(), actual.type_name);

    for (const auto& pair : schema.fields()) {
        field_descriptor_t descriptor = pair.second;

        bool found = find(begin(actual.descriptors),
                          end(actual.descriptors),
                          descriptor) != end(actual.descriptors);

        EXPECT_TRUE(found);
    }
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast
