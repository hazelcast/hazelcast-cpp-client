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

#include "hazelcast/client/serialization/pimpl/compact/schema.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

class HAZELCAST_API schema_writer
{
public:
    explicit schema_writer(std::string type_name);
    void add_field(std::string field_name, enum field_kind kind);
    schema build() &&;

private:
    std::unordered_map<std::string, field_descriptor> field_definition_map_;
    std::string type_name_;
};

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast