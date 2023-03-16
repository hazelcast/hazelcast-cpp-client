
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

#include <unordered_map>
#include <string>
#include <iosfwd>

#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/pimpl/compact/field_descriptor.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

class HAZELCAST_API schema
{
public:
    schema() = default;
    schema(
      std::string type_name,
      std::unordered_map<std::string, field_descriptor>&& field_definition_map);
    int64_t schema_id() const;
    size_t number_of_var_size_fields() const;
    size_t fixed_size_fields_length() const;
    const std::string& type_name() const;
    const std::unordered_map<std::string, field_descriptor>& fields() const;
    boost::optional<field_descriptor> get_field(
      const std::string& field_name) const;

private:
    std::string type_name_;
    std::unordered_map<std::string, field_descriptor> field_definition_map_;
    size_t number_of_var_size_fields_{};
    size_t fixed_size_fields_length_{};
    int64_t schema_id_{};
};

bool HAZELCAST_API
operator==(const schema& x, const schema& y);

bool HAZELCAST_API
operator!=(const schema& x, const schema& y);

std::ostream HAZELCAST_API&
operator<<(std::ostream& os, const schema& schema);

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
