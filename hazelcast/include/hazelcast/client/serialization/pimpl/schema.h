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
#pragma once

#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/field_kind.h"

#include <string>
#include <unordered_map>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

struct HAZELCAST_API field_descriptor
{
    enum field_kind field_kind;
    /**
     * Index of the offset of the non-primitive field. For others, it is -1
     */
    int32_t index;
    /**
     * Applicable only for primitive fields. For others, it is -1
     */
    int32_t offset;
    /**
     * Applicable only for boolean field. For others, it is -1
     */
    int8_t bit_offset;
};

std::ostream&
operator<<(std::ostream& os, const field_descriptor& field_descriptor);

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

private:
    std::string type_name_;
    std::unordered_map<std::string, field_descriptor> field_definition_map_;
    size_t number_of_var_size_fields_{};
    size_t fixed_size_fields_length_{};
    int64_t schema_id_{};
};

std::ostream&
operator<<(std::ostream& os, const schema& schema);
} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
