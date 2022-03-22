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
//#include <boost/concept_check.hpp>
//#include <utility>

#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

schema::schema() {

}

schema::schema(const std::string& type_name,
               std::map<std::string, field_descriptor>&& field_definition_map)
  : type_name_(type_name)
  , field_definition_map(field_definition_map)
{}

long
schema::schema_id() const
{
    return schema_id_;
}

boost::future<schema>
default_schema_service::get(long schemaId)
{
    auto ptr = schemas.get(schemaId);
    if (ptr == nullptr) {
        return boost::make_ready_future<schema>();
    }
    return boost::make_ready_future(*ptr);
}
boost::future<void>
default_schema_service::put(std::shared_ptr<schema> schema_ptr)
{
    schemas.put(schema_ptr->schema_id(), schema_ptr);
    return boost::make_ready_future();
}



} // namespace pimpl

void
compact_writer::write_int32(const std::string& field_name, int32_t value)
{}
void
compact_writer::write_string(const std::string& field_name,
                             const boost::optional<std::string>& value)
{}

int32_t
compact_reader::read_int32(const std::string& field_name)
{
    return 0;
}

int32_t
compact_reader::read_int32(const std::string& field_name, int32_t default_value)
{
    return default_value;
}

boost::optional<std::string>
compact_reader::read_string(const std::string& field_name)
{
    return boost::none;
}
boost::optional<std::string>
compact_reader::read_string(const std::string& field_name,
                            const boost::optional<std::string>& default_value)
{
    return default_value;
}
} // namespace serialization
} // namespace client
} // namespace hazelcast
