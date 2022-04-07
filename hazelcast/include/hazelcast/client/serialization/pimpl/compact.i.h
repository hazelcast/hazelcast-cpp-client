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
#pragma once

#include "hazelcast/client/serialization/pimpl/compact.h"
#include "hazelcast/util/finally.h"

namespace hazelcast {
namespace client {
namespace serialization {

namespace pimpl {
namespace offset_reader {

template<typename OFFSET_TYPE>
int32_t
get_offset(serialization::object_data_input& in,
           uint32_t variable_offsets_pos,
           uint32_t index)
{
    OFFSET_TYPE offset = in.read<OFFSET_TYPE>(variable_offsets_pos + index);
    if (offset == NULL_OFFSET) {
        return NULL_OFFSET;
    }
    return (int32_t)offset;
}

} // namespace offset_reader
} // namespace pimpl
template<typename T>
boost::optional<T>
compact_reader::get_variable_size(
  const pimpl::field_descriptor& field_descriptor)
{
    int current_pos = object_data_input.position();
    util::finally set_position_back(
      [this, current_pos]() { this->object_data_input.position(current_pos); });
    int pos = read_var_size_position(field_descriptor);
    if (pos == pimpl::offset_reader::NULL_OFFSET) {
        return boost::none;
    }
    object_data_input.position(pos);
    return object_data_input.read<T>();
}

template<typename T>
boost::optional<T>
compact_reader::get_variable_size(const std::string& field_name,
                                  enum pimpl::field_kind field_kind)
{
    auto field_descriptor = get_field_descriptor(field_name, field_kind);
    return get_variable_size<T>(field_descriptor);
}

template<typename T>
T
compact_reader::get_variable_as_non_null(
  const pimpl::field_descriptor& field_descriptor,
  const std::string& field_name,
  const std::string& method_suffix)
{
    auto value = get_variable_size<int32_t>(field_descriptor);
    if (value.has_value()) {
        return value.value();
    }
    BOOST_THROW_EXCEPTION(unexpected_null_value(field_name, method_suffix));
}

namespace pimpl {

template<typename T>
void
default_compact_writer::write_variable_size_field(
  const std::string& field_name,
  enum field_kind field_kind,
  const boost::optional<T>& value)
{
    if (!value.has_value()) {
        set_position_as_null(field_name, field_kind);
    } else {
        set_position(field_name, field_kind);
        object_data_output_.write<T>(value.value());
    }
}

template<typename T>
struct schema_of
{
    static schema build_schema()
    {
        T t;
        schema_writer schema_writer(hz_serializer<T>::type_name());
        serialization::compact_writer writer =
          create_compact_writer(&schema_writer);
        serialization::hz_serializer<T>::write(t, writer);
        return std::move(schema_writer).build();
    }

    const static schema schema_v;
};

template<typename T>
const schema schema_of<T>::schema_v = schema_of<T>::build_schema();

template<typename T>
T inline compact_stream_serializer::read(object_data_input& in)
{
    int64_t schema_id = in.read<int64_t>();
    auto schema = schema_service.get(schema_id);
    compact_reader reader = create_compact_reader(*this, in, schema);
    return hz_serializer<T>::read(reader);
}

template<typename T>
void inline compact_stream_serializer::write(const T& object,
                                             object_data_output& out)
{
    const auto& schema_v = schema_of<T>::schema_v;
    put_to_schema_service(schema_v);
    out.write<int64_t>(schema_v.schema_id());
    default_compact_writer default_writer(*this, out, schema_v);
    compact_writer writer = create_compact_writer(&default_writer);
    hz_serializer<T>::write(object, writer);
    default_writer.end();
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast