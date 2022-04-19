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
#include <type_traits>

namespace hazelcast {
namespace client {
namespace serialization {

namespace pimpl {
namespace offset_reader {

/**
 * Returns the offset of the variable-size field at the given index.
 *
 * @tparam OFFSET_TYPE can be int32_t, int16_t or int8_t
 * @param in Input to read the offset from.
 * @param variableOffsetsPos Start of the variable-size field offsets
 *                           section of the input.
 * @param index Index of the field.
 * @return The offset.
 */
template<typename OFFSET_TYPE>
int32_t
get_offset(serialization::object_data_input& in,
           uint32_t variable_offsets_pos,
           uint32_t index)
{
    OFFSET_TYPE v = in.read<OFFSET_TYPE>(variable_offsets_pos +
                                         (index * sizeof(OFFSET_TYPE)));
    if (v == NULL_OFFSET) {
        return NULL_OFFSET;
    }
    return (int32_t)(typename std::make_unsigned<OFFSET_TYPE>::type)(v);
}

} // namespace offset_reader
} // namespace pimpl

template<typename T>
T
compact_reader::read_primitive(const std::string& field_name,
                               enum pimpl::field_kind primitive_field_kind,
                               enum pimpl::field_kind nullable_field_kind,
                               const std::string& method_suffix)
{
    const auto& fd = get_field_descriptor(field_name);
    const auto& field_kind = fd.field_kind;
    if (field_kind == primitive_field_kind) {
        return read_primitive<T>(fd);
    } else if (field_kind == nullable_field_kind) {
        return read_variable_size_as_non_null<T>(fd, field_name, method_suffix);
    } else {
        BOOST_THROW_EXCEPTION(unexpected_field_kind(field_kind, field_name));
    }
}

template<typename T>
T inline compact_reader::read_primitive(
  const pimpl::field_descriptor& field_descriptor)
{
    return object_data_input.read<T>(
      read_fixed_size_position(field_descriptor));
}

template<>
bool inline compact_reader::read_primitive<bool>(
  const pimpl::field_descriptor& field_descriptor)
{
    int32_t boolean_offset = field_descriptor.offset;
    int8_t bit_offset = field_descriptor.bit_offset;
    int32_t offset = boolean_offset + data_start_position;
    byte last_byte = object_data_input.read<byte>(offset);
    return ((last_byte >> bit_offset) & 1) != 0;
}

template<typename T>
boost::optional<T>
compact_reader::read_variable_size(
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
    return read<T>();
}

template<typename T>
boost::optional<T>
compact_reader::read_variable_size(const std::string& field_name,
                                   enum pimpl::field_kind field_kind)
{
    auto field_descriptor = get_field_descriptor(field_name, field_kind);
    return read_variable_size<T>(field_descriptor);
}

template<typename T>
T
compact_reader::read_variable_size_as_non_null(
  const pimpl::field_descriptor& field_descriptor,
  const std::string& field_name,
  const std::string& method_suffix)
{
    auto value = read_variable_size<T>(field_descriptor);
    if (value.has_value()) {
        return value.value();
    }
    BOOST_THROW_EXCEPTION(unexpected_null_value(field_name, method_suffix));
}

template<typename T>
typename std::enable_if<
  std::is_same<bool, typename std::remove_cv<T>::type>::value ||
    std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<float, typename std::remove_cv<T>::type>::value ||
    std::is_same<double, typename std::remove_cv<T>::type>::value ||
    std::is_same<std::string, typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<int8_t>,
                 typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<int16_t>,
                 typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<int32_t>,
                 typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<int64_t>,
                 typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<double>,
                 typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<boost::optional<std::string>>,
                 typename std::remove_cv<T>::type>::value,
  typename boost::optional<T>>::type
compact_reader::read()
{
    return object_data_input.template read<T>();
}

template<typename T>
typename std::enable_if<
  std::is_base_of<compact_serializer, hz_serializer<T>>::value,
  typename boost::optional<T>>::type
compact_reader::read()
{
    return compact_stream_serializer.template read<T>(object_data_input);
}

template<typename T>
typename std::enable_if<
  std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value,
  typename boost::optional<T>>::type
compact_reader::read()
{
    int32_t len = object_data_input.read<int32_t>();
    if (len == 0) {
        return boost::make_optional(std::vector<bool>(0));
    }
    std::vector<bool> values(len);
    int index = 0;
    byte current_byte = object_data_input.read<byte>();
    for (int i = 0; i < len; ++i) {
        if (index == util::Bits::BITS_IN_BYTE) {
            index = 0;
            current_byte = object_data_input.read<byte>();
        }
        bool result = ((current_byte >> index) & 1) != 0;
        index++;
        values[i] = result;
    }
    return boost::make_optional(std::move(values));
}

template<typename T>
typename std::enable_if<std::is_same<std::vector<boost::optional<bool>>,
                                     typename std::remove_cv<T>::type>::value,
                        typename boost::optional<T>>::type
compact_reader::read()
{
    int32_t len = object_data_input.read<int32_t>();
    if (len == 0) {
        return boost::make_optional<std::vector<boost::optional<bool>>>(
          std::vector<boost::optional<bool>>(0));
    }
    auto values = boost::make_optional<std::vector<boost::optional<bool>>>(
      std::vector<boost::optional<bool>>(len));
    int index = 0;
    byte current_byte = object_data_input.read<byte>();
    for (int i = 0; i < len; ++i) {
        if (index == util::Bits::BITS_IN_BYTE) {
            index = 0;
            current_byte = object_data_input.read<byte>();
        }
        values.value()[i] =
          boost::make_optional<bool>(((current_byte >> index) & 1) != 0);
        index++;
    }
    return values;
}

template<typename T>
boost::optional<T>
compact_reader::read_array_of_primitive(
  const std::string& field_name,
  enum pimpl::field_kind field_kind,
  enum pimpl::field_kind nullable_field_kind,
  const std::string& method_suffix)
{
    auto& field_descriptor = get_field_descriptor(field_name);
    if (field_descriptor.field_kind == field_kind) {
        return read_variable_size<T>(field_descriptor);
    } else if (field_descriptor.field_kind == nullable_field_kind) {
        return read_nullable_array_as_primitive_array<T>(
          field_descriptor, field_name, method_suffix);
    }
    throw unexpected_field_kind(field_descriptor.field_kind, field_name);
}

template<typename T>
boost::optional<std::vector<boost::optional<T>>>
compact_reader::read_array_of_variable_size(
  const pimpl::field_descriptor& field_descriptor)
{
    int current_pos = object_data_input.position();
    util::finally set_position_back(
      [this, current_pos]() { this->object_data_input.position(current_pos); });

    int32_t position = read_var_size_position(field_descriptor);
    if (position == util::Bits::NULL_ARRAY) {
        return boost::none;
    }
    object_data_input.position(position);
    int32_t data_length = object_data_input.read<int32_t>();
    int32_t item_count = object_data_input.read<int32_t>();
    int data_start_pos = object_data_input.position();
    std::vector<boost::optional<T>> values(item_count);
    auto offset_reader = get_offset_reader(data_length);
    int offsets_position = data_start_pos + data_length;
    for (int i = 0; i < item_count; ++i) {
        int offset = offset_reader(object_data_input, offsets_position, i);
        if (offset != util::Bits::NULL_ARRAY) {
            object_data_input.position(offset + data_start_pos);
            values[i] = read<T>();
        }
    }
    return values;
}

template<typename T>
boost::optional<T>
compact_reader::read_nullable_array_as_primitive_array(
  const pimpl::field_descriptor& field_descriptor,
  const std::string& field_name,
  const std::string& method_suffix)
{
    int current_pos = object_data_input.position();
    util::finally set_position_back(
      [this, current_pos]() { this->object_data_input.position(current_pos); });

    int32_t position = read_var_size_position(field_descriptor);
    if (position == util::Bits::NULL_ARRAY) {
        return boost::none;
    }
    object_data_input.position(position);
    int32_t data_length = object_data_input.read<int32_t>();
    int32_t item_count = object_data_input.read<int32_t>();
    int data_start_pos = object_data_input.position();
    auto offset_reader = get_offset_reader(data_length);
    int offsets_position = data_start_pos + data_length;
    for (int i = 0; i < item_count; ++i) {
        int offset = offset_reader(object_data_input, offsets_position, i);
        if (offset == util::Bits::NULL_ARRAY) {
            BOOST_THROW_EXCEPTION(
              unexpected_null_value_in_array(field_name, method_suffix));
        }
    }
    object_data_input.position(data_start_pos - util::Bits::INT_SIZE_IN_BYTES);
    return read<T>();
}

template<typename T>
boost::optional<T>
compact_reader::read_nullable_primitive(
  const std::string& field_name,
  enum pimpl::field_kind field_kind,
  enum pimpl::field_kind nullable_field_kind)
{
    auto& field_descriptor = get_field_descriptor(field_name);
    if (field_descriptor.field_kind == field_kind) {
        return boost::make_optional<T>(read_primitive<T>(field_descriptor));
    } else if (field_descriptor.field_kind == nullable_field_kind) {
        return read_variable_size<T>(field_descriptor);
    }
    throw unexpected_field_kind(field_descriptor.field_kind, field_name);
}

template<typename T>
boost::optional<std::vector<boost::optional<T>>>
compact_reader::read_array_of_nullable(
  const std::string& field_name,
  enum pimpl::field_kind field_kind,
  enum pimpl::field_kind nullable_field_kind)
{
    auto& field_descriptor = get_field_descriptor(field_name);
    if (field_descriptor.field_kind == field_kind) {
        return read_primitive_array_as_nullable_array<T>(field_descriptor);
    } else if (field_descriptor.field_kind == nullable_field_kind) {
        return read_array_of_variable_size<T>(field_descriptor);
    }
    throw unexpected_field_kind(field_descriptor.field_kind, field_name);
}

template<typename T>
boost::optional<std::vector<boost::optional<T>>>
compact_reader::read_primitive_array_as_nullable_array(
  const pimpl::field_descriptor& field_descriptor)
{
    int32_t current_pos = object_data_input.position();
    util::finally set_position_back(
      [this, current_pos]() { this->object_data_input.position(current_pos); });

    int32_t pos = read_var_size_position(field_descriptor);
    if (pos == pimpl::offset_reader::NULL_OFFSET) {
        return boost::none;
    }
    object_data_input.position(pos);
    int32_t item_count = object_data_input.read<int32_t>();
    auto values = boost::make_optional<std::vector<boost::optional<T>>>(
      std::vector<boost::optional<T>>(item_count));

    for (int i = 0; i < item_count; ++i) {
        values.value()[i] = boost::make_optional(object_data_input.read<T>());
    }
    return values;
}

template<>
boost::optional<std::vector<boost::optional<bool>>> inline compact_reader::
  read_primitive_array_as_nullable_array(
    const pimpl::field_descriptor& field_descriptor)
{
    return read_variable_size<std::vector<boost::optional<bool>>>(
      field_descriptor);
}

template<typename T>
boost::optional<T>
compact_reader::read_compact(const std::string& field_name)
{
    return read_variable_size<T>(field_name, pimpl::field_kind::COMPACT);
}

template<typename T>
boost::optional<std::vector<boost::optional<T>>>
compact_reader::read_array_of_compact(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, pimpl::field_kind::ARRAY_OF_COMPACT);
    return read_array_of_variable_size<T>(descriptor);
}

template<typename T>
void
compact_writer::write_compact(const std::string& field_name,
                              const boost::optional<T>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_compact(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::COMPACT);
    }
}

template<typename T>
void
compact_writer::write_array_of_compact(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<T>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_compact<T>(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::ARRAY_OF_COMPACT);
    }
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
        write<T>(value.value());
    }
}

template<typename T>
void
default_compact_writer::write_compact(const std::string& field_name,
                                      const boost::optional<T>& value)
{
    write_variable_size_field<T>(field_name, field_kind::COMPACT, value);
}

template<typename T>
void
default_compact_writer::write_array_of_compact(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<T>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_COMPACT, value);
}

template<typename T>
void
default_compact_writer::write_array_of_variable_size(
  const std::string& field_name,
  enum field_kind field_kind,
  const boost::optional<std::vector<boost::optional<T>>>& value)
{
    if (!value.has_value()) {
        set_position_as_null(field_name, field_kind);
        return;
    }
    set_position(field_name, field_kind);
    size_t data_length_offset = object_data_output_.position();
    object_data_output_.write_zero_bytes(util::Bits::INT_SIZE_IN_BYTES);
    const auto& v = value.value();
    int item_count = v.size();
    object_data_output_.write<int32_t>(item_count);
    size_t offset = object_data_output_.position();
    std::vector<int32_t> offsets(item_count);
    for (int i = 0; i < item_count; ++i) {
        if (v[i].has_value()) {
            offsets[i] =
              static_cast<int32_t>(object_data_output_.position() - offset);
            write<T>(v[i].value());
        } else {
            offsets[i] = -1;
        }
    }
    auto data_length =
      static_cast<int32_t>(object_data_output_.position() - offset);
    object_data_output_.write_at<int32_t>(data_length_offset, data_length);
    write_offsets(data_length, offsets);
}
template<typename T>
typename std::enable_if<
  std::is_same<bool, typename std::remove_cv<T>::type>::value ||
    std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
    std::is_same<float, typename std::remove_cv<T>::type>::value ||
    std::is_same<double, typename std::remove_cv<T>::type>::value ||
    std::is_same<std::string, typename std::remove_cv<T>::type>::value ||
    std::is_same<std::vector<int8_t>, T>::value ||
    std::is_same<std::vector<int16_t>, T>::value ||
    std::is_same<std::vector<int32_t>, T>::value ||
    std::is_same<std::vector<int64_t>, T>::value ||
    std::is_same<std::vector<float>, T>::value ||
    std::is_same<std::vector<double>, T>::value ||
    std::is_same<std::vector<std::string>, T>::value,
  void>::type
default_compact_writer::write(const T& value)
{
    object_data_output_.write(value);
}

template<typename T>
typename std::enable_if<
  std::is_base_of<compact_serializer, hz_serializer<T>>::value,
  void>::type
default_compact_writer::write(const T& value)
{
    compact_stream_serializer_.template write<T>(value, object_data_output_);
}

template<typename T>
typename std::enable_if<std::is_same<std::vector<bool>, T>::value, void>::type
default_compact_writer::write(const T& value)
{
    auto len = value.size();
    object_data_output_.write<int32_t>(len);
    size_t position = object_data_output_.position();
    if (len > 0) {
        int index = 0;
        object_data_output_.write_zero_bytes(1);
        for (auto v : value) {
            if (index == util::Bits::BITS_IN_BYTE) {
                index = 0;
                object_data_output_.write_zero_bytes(1);
                position++;
            }
            object_data_output_.write_boolean_bit_at(position, index, v);
            index++;
        }
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
    const auto& local_schema = schema_of<T>::schema_v;
    // optimization to avoid hitting shared map in the schema_service,
    // in the case incoming data's schema is same as the local schema
    if (schema_id == local_schema.schema_id()) {
        compact_reader reader = create_compact_reader(*this, in, local_schema);
        return hz_serializer<T>::read(reader);
    }
    // This path will run only in schema evolution case
    auto schema = schema_service.get(schema_id);
    if (schema.type_name() != hz_serializer<T>::type_name()) {
        auto exception = exception::hazelcast_serialization{
            "compact_stream_serializer",
            (boost::format("Unexpected typename. expected %1%, received %2%") %
             hz_serializer<T>::type_name() % schema.type_name())
              .str()
        };
        BOOST_THROW_EXCEPTION(exception);
    }
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