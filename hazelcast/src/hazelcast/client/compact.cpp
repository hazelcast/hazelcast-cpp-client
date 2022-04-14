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

#include <utility>
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Bits.h"
namespace hazelcast {
namespace client {
namespace serialization {

compact_writer::compact_writer(
  pimpl::default_compact_writer* default_compact_writer)
  : default_compact_writer(default_compact_writer)
  , schema_writer(nullptr)
{}
compact_writer::compact_writer(pimpl::schema_writer* schema_writer)
  : default_compact_writer(nullptr)
  , schema_writer(schema_writer)
{}

void
compact_writer::write_boolean(const std::string& field_name, bool value)
{
    if (default_compact_writer) {
        default_compact_writer->write_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::BOOLEAN);
    }
}
void
compact_writer::write_int8(const std::string& field_name, int8_t value)
{
    if (default_compact_writer) {
        default_compact_writer->write_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::INT8);
    }
}

void
compact_writer::write_int16(const std::string& field_name, int16_t value)
{
    if (default_compact_writer) {
        default_compact_writer->write_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::INT16);
    }
}

void
compact_writer::write_int32(const std::string& field_name, int32_t value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::INT32);
    }
}

void
compact_writer::write_int64(const std::string& field_name, int64_t value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::INT64);
    }
}

void
compact_writer::write_float32(const std::string& field_name, float value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::FLOAT32);
    }
}

void
compact_writer::write_float64(const std::string& field_name, double value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::FLOAT64);
    }
}

void
compact_writer::write_string(const std::string& field_name,
                             const boost::optional<std::string>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_string(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::STRING);
    }
}

void
compact_writer::write_array_of_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<bool>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::ARRAY_OF_BOOLEAN);
    }
}

void
compact_writer::write_array_of_int8(
  const std::string& field_name,
  const boost::optional<std::vector<int8_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::ARRAY_OF_INT8);
    }
}

void
compact_writer::write_array_of_int16(
  const std::string& field_name,
  const boost::optional<std::vector<int16_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::ARRAY_OF_INT16);
    }
}

void
compact_writer::write_array_of_int32(
  const std::string& field_name,
  const boost::optional<std::vector<int32_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::ARRAY_OF_INT32);
    }
}

void
compact_writer::write_array_of_int64(
  const std::string& field_name,
  const boost::optional<std::vector<int64_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::ARRAY_OF_INT64);
    }
}

void
compact_writer::write_array_of_float32(
  const std::string& field_name,
  const boost::optional<std::vector<float>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::ARRAY_OF_FLOAT32);
    }
}

void
compact_writer::write_array_of_float64(
  const std::string& field_name,
  const boost::optional<std::vector<double>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::ARRAY_OF_FLOAT64);
    }
}

void
compact_writer::write_array_of_string(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<std::string>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_string(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::ARRAY_OF_STRING);
    }
}

void
compact_writer::write_nullable_boolean(const std::string& field_name,
                                       const boost::optional<bool>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::NULLABLE_BOOLEAN);
    }
}

void
compact_writer::write_nullable_int8(const std::string& field_name,
                                    const boost::optional<int8_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::NULLABLE_INT8);
    }
}

void
compact_writer::write_nullable_int16(const std::string& field_name,
                                     const boost::optional<int16_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::NULLABLE_INT16);
    }
}

void
compact_writer::write_nullable_int32(const std::string& field_name,
                                     const boost::optional<int32_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::NULLABLE_INT32);
    }
}

void
compact_writer::write_nullable_int64(const std::string& field_name,
                                     const boost::optional<int64_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::NULLABLE_INT64);
    }
}

void
compact_writer::write_nullable_float32(const std::string& field_name,
                                       const boost::optional<float>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::NULLABLE_FLOAT32);
    }
}

void
compact_writer::write_nullable_float64(const std::string& field_name,
                                       const boost::optional<double>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 pimpl::field_kind::NULLABLE_FLOAT64);
    }
}

namespace pimpl {
compact_reader
create_compact_reader(
  pimpl::compact_stream_serializer& compact_stream_serializer,
  object_data_input& object_data_input,
  const pimpl::schema& schema)
{
    return compact_reader{ compact_stream_serializer,
                           object_data_input,
                           schema };
}
} // namespace pimpl

const compact_reader::offset_func compact_reader::BYTE_OFFSET_READER =
  pimpl::offset_reader::get_offset<int8_t>;
const compact_reader::offset_func compact_reader::SHORT_OFFSET_READER =
  pimpl::offset_reader::get_offset<int16_t>;
const compact_reader::offset_func compact_reader::INT_OFFSET_READER =
  pimpl::offset_reader::get_offset<int32_t>;

compact_reader::compact_reader(
  pimpl::compact_stream_serializer& compact_stream_serializer,
  serialization::object_data_input& object_data_input,
  const pimpl::schema& schema)
  : compact_stream_serializer(compact_stream_serializer)
  , object_data_input(object_data_input)
  , schema(schema)
{
    size_t final_position;
    size_t number_of_var_size_fields = schema.number_of_var_size_fields();
    if (number_of_var_size_fields != 0) {
        uint32_t data_length = object_data_input.read<int32_t>();
        data_start_position = object_data_input.position();
        variable_offsets_position = data_start_position + data_length;
        if (data_length < pimpl::offset_reader::BYTE_OFFSET_READER_RANGE) {
            get_offset = BYTE_OFFSET_READER;
            final_position =
              variable_offsets_position + number_of_var_size_fields;
        } else if (data_length <
                   pimpl::offset_reader::SHORT_OFFSET_READER_RANGE) {
            get_offset = SHORT_OFFSET_READER;
            final_position =
              variable_offsets_position +
              (number_of_var_size_fields * util::Bits::SHORT_SIZE_IN_BYTES);
        } else {
            get_offset = INT_OFFSET_READER;
            final_position =
              variable_offsets_position +
              (number_of_var_size_fields * util::Bits::INT_SIZE_IN_BYTES);
        }
    } else {
        get_offset = INT_OFFSET_READER;
        variable_offsets_position = 0;
        data_start_position = object_data_input.position();
        final_position =
          data_start_position + schema.fixed_size_fields_length();
    }
    // set the position to final so that the next one to read something from
    // `in` can start from correct position
    object_data_input.position(static_cast<int>(final_position));
}

bool
compact_reader::is_field_exists(const std::string& field_name,
                                enum pimpl::field_kind kind) const
{
    const auto& fields = schema.fields();
    const auto& field_descriptor = fields.find(field_name);
    if (field_descriptor == fields.end()) {
        return false;
    }
    return field_descriptor->second.field_kind == kind;
}

const pimpl::field_descriptor&
compact_reader::get_field_descriptor(const std::string& field_name) const
{
    const auto& fields = schema.fields();
    const auto& field_descriptor = fields.find(field_name);
    if (field_descriptor == fields.end()) {
        BOOST_THROW_EXCEPTION(unknown_field(field_name));
    }
    return field_descriptor->second;
}

const pimpl::field_descriptor&
compact_reader::get_field_descriptor(const std::string& field_name,
                                     enum pimpl::field_kind kind) const
{
    const auto& field_descriptor = get_field_descriptor(field_name);
    if (field_descriptor.field_kind != kind) {
        BOOST_THROW_EXCEPTION(
          unexpected_field_kind(field_descriptor.field_kind, field_name));
    }
    return field_descriptor;
}

std::function<int32_t(serialization::object_data_input&, uint32_t, uint32_t)>
compact_reader::get_offset_reader(int32_t data_length)
{
    if (data_length < pimpl::offset_reader::BYTE_OFFSET_READER_RANGE) {
        return BYTE_OFFSET_READER;
    } else if (data_length < pimpl::offset_reader::SHORT_OFFSET_READER_RANGE) {
        return SHORT_OFFSET_READER;
    } else {
        return INT_OFFSET_READER;
    }
}

exception::hazelcast_serialization
compact_reader::unexpected_null_value_in_array(const std::string& field_name,
                                               const std::string& method_suffix)
{
    return {
        "compact_reader",
        (boost::format(
           "Error while reading %1%. Array with null items can not be read via "
           "read_array_of_%2% methods. Use read_array_of_nullable_%2% "
           "instead") %
         field_name % method_suffix)
          .str()
    };
}
exception::hazelcast_serialization
compact_reader::unknown_field(const std::string& field_name) const
{
    return { "compact_reader",
             (boost::format("Unknown field name %1% on %2% ") % field_name %
              schema)
               .str() };
}

exception::hazelcast_serialization
compact_reader::unexpected_field_kind(enum pimpl::field_kind field_kind,
                                      const std::string& field_name) const
{
    return { "compact_reader",
             (boost::format("Unexpected fieldKind %1% for %2% on %3%") %
              field_kind % field_name % schema)
               .str() };
}

exception::hazelcast_serialization
compact_reader::unexpected_null_value(const std::string& field_name,
                                      const std::string& method_suffix)
{
    return { "compact_reader",
             (boost::format(
                "Error while reading %1%. null value can not be read via "
                "get_%2% methods. Use get_nullable_%2%  instead.") %
              field_name % method_suffix)
               .str() };
}

size_t
compact_reader::read_fixed_size_position(
  const pimpl::field_descriptor& field_descriptor) const
{
    int32_t primitive_offset = field_descriptor.offset;
    return primitive_offset + data_start_position;
}

int32_t
compact_reader::read_var_size_position(
  const pimpl::field_descriptor& field_descriptor) const
{
    int32_t index = field_descriptor.index;
    int32_t offset =
      get_offset(object_data_input, variable_offsets_position, index);
    return offset == pimpl::offset_reader::NULL_OFFSET
             ? pimpl::offset_reader::NULL_OFFSET
             : offset + data_start_position;
}

bool
compact_reader::read_boolean(const std::string& fieldName)
{
    return read_primitive<bool>(fieldName,
                                pimpl::field_kind::BOOLEAN,
                                pimpl::field_kind::NULLABLE_BOOLEAN,
                                "boolean");
}

int8_t
compact_reader::read_int8(const std::string& fieldName)
{
    return read_primitive<int8_t>(fieldName,
                                  pimpl::field_kind::INT8,
                                  pimpl::field_kind::NULLABLE_INT8,
                                  "int8");
}

int16_t
compact_reader::read_int16(const std::string& field_name)
{
    return read_primitive<int16_t>(field_name,
                                   pimpl::field_kind::INT16,
                                   pimpl::field_kind::NULLABLE_INT16,
                                   "int16");
}

int32_t
compact_reader::read_int32(const std::string& field_name)
{
    return read_primitive<int32_t>(field_name,
                                   pimpl::field_kind::INT32,
                                   pimpl::field_kind::NULLABLE_INT32,
                                   "int32");
}

int64_t
compact_reader::read_int64(const std::string& field_name)
{
    return read_primitive<int64_t>(field_name,
                                   pimpl::field_kind::INT64,
                                   pimpl::field_kind::NULLABLE_INT64,
                                   "int64");
}

float
compact_reader::read_float32(const std::string& field_name)
{
    return read_primitive<float>(field_name,
                                 pimpl::field_kind::FLOAT32,
                                 pimpl::field_kind::NULLABLE_FLOAT32,
                                 "float32");
}

double
compact_reader::read_float64(const std::string& field_name)
{
    return read_primitive<double>(field_name,
                                  pimpl::field_kind::FLOAT64,
                                  pimpl::field_kind::NULLABLE_FLOAT64,
                                  "float64");
}

boost::optional<std::string>
compact_reader::read_string(const std::string& field_name)
{
    return read_variable_size<std::string>(field_name,
                                           pimpl::field_kind::STRING);
}

boost::optional<std::vector<bool>>
compact_reader::read_array_of_boolean(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<bool>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_BOOLEAN,
      pimpl::ARRAY_OF_NULLABLE_BOOLEAN,
      "boolean");
}

boost::optional<std::vector<int8_t>>
compact_reader::read_array_of_int8(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int8_t>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_INT8,
      pimpl::ARRAY_OF_NULLABLE_INT8,
      "int8");
}

boost::optional<std::vector<int16_t>>
compact_reader::read_array_of_int16(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int16_t>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_INT16,
      pimpl::ARRAY_OF_NULLABLE_INT16,
      "int16");
}

boost::optional<std::vector<int32_t>>
compact_reader::read_array_of_int32(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int32_t>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_INT32,
      pimpl::ARRAY_OF_NULLABLE_INT32,
      "int32");
}
boost::optional<std::vector<int64_t>>
compact_reader::read_array_of_int64(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int64_t>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_INT64,
      pimpl::ARRAY_OF_NULLABLE_INT64,
      "int64");
}

boost::optional<std::vector<float>>
compact_reader::read_array_of_float32(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<float>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_FLOAT32,
      pimpl::ARRAY_OF_NULLABLE_FLOAT32,
      "float32");
}

boost::optional<std::vector<double>>
compact_reader::read_array_of_float64(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<double>>(
      field_name,
      pimpl::field_kind::ARRAY_OF_FLOAT64,
      pimpl::ARRAY_OF_NULLABLE_FLOAT64,
      "float64");
}

boost::optional<std::vector<boost::optional<std::string>>>
compact_reader::read_array_of_string(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, pimpl::field_kind::ARRAY_OF_STRING);
    return read_array_of_variable_size<std::string>(descriptor);
}

boost::optional<bool>
compact_reader::read_nullable_boolean(const std::string& field_name)
{
    return get_nullable_primitive<bool>(field_name,
                                        pimpl::field_kind::BOOLEAN,
                                        pimpl::field_kind::NULLABLE_BOOLEAN);
}

boost::optional<int8_t>
compact_reader::read_nullable_int8(const std::string& field_name)
{
    return get_nullable_primitive<int8_t>(
      field_name, pimpl::field_kind::INT8, pimpl::field_kind::NULLABLE_INT8);
}

boost::optional<int16_t>
compact_reader::read_nullable_int16(const std::string& field_name)
{
    return get_nullable_primitive<int16_t>(
      field_name, pimpl::field_kind::INT16, pimpl::field_kind::NULLABLE_INT16);
}

boost::optional<int32_t>
compact_reader::read_nullable_int32(const std::string& field_name)
{
    return get_nullable_primitive<int32_t>(
      field_name, pimpl::field_kind::INT32, pimpl::field_kind::NULLABLE_INT32);
}

boost::optional<int64_t>
compact_reader::read_nullable_int64(const std::string& field_name)
{
    return get_nullable_primitive<int64_t>(
      field_name, pimpl::field_kind::INT64, pimpl::field_kind::NULLABLE_INT64);
}

boost::optional<float>
compact_reader::read_nullable_float32(const std::string& field_name)
{
    return get_nullable_primitive<float>(field_name,
                                         pimpl::field_kind::FLOAT32,
                                         pimpl::field_kind::NULLABLE_FLOAT32);
}

boost::optional<double>
compact_reader::read_nullable_float64(const std::string& field_name)
{
    return get_nullable_primitive<double>(field_name,
                                          pimpl::field_kind::FLOAT64,
                                          pimpl::field_kind::NULLABLE_FLOAT64);
}

namespace pimpl {

compact_writer
create_compact_writer(pimpl::default_compact_writer* default_compact_writer)
{
    return compact_writer{ default_compact_writer };
}
compact_writer
create_compact_writer(pimpl::schema_writer* schema_writer)
{
    return compact_writer{ schema_writer };
}

default_compact_writer::default_compact_writer(
  compact_stream_serializer& compact_stream_serializer,
  object_data_output& object_data_output,
  const schema& schema)
  : compact_stream_serializer_(compact_stream_serializer)
  , object_data_output_(object_data_output)
  , schema_(schema)
  , field_offsets(schema.number_of_var_size_fields())
{
    if (schema.number_of_var_size_fields() != 0) {
        data_start_position =
          object_data_output_.position() + util::Bits::INT_SIZE_IN_BYTES;
        // Skip for length and primitives.
        object_data_output_.write_zero_bytes(schema.fixed_size_fields_length() +
                                             util::Bits::INT_SIZE_IN_BYTES);
    } else {
        data_start_position = object_data_output_.position();
        // Skip for primitives. No need to write data length, when there is no
        // variable-size fields.
        object_data_output_.write_zero_bytes(schema.fixed_size_fields_length());
    }
}

void
default_compact_writer::write_boolean(const std::string& field_name, bool value)
{
    field_descriptor descriptor =
      check_field_definition(field_name, field_kind::BOOLEAN);
    int32_t offset_in_bytes = descriptor.offset;
    int8_t offset_in_bits = descriptor.bit_offset;
    size_t write_offset = offset_in_bytes + data_start_position;
    object_data_output_.write_boolean_bit_at(
      write_offset, offset_in_bits, value);
}

void
default_compact_writer::write_int8(const std::string& field_name, int8_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT8);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int16(const std::string& field_name,
                                    int16_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT16);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int32(const std::string& field_name,
                                    int32_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT32);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int64(const std::string& field_name,
                                    int64_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT64);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_float32(const std::string& field_name,
                                      float value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::FLOAT32);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_float64(const std::string& field_name,
                                      double value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::FLOAT64);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_string(const std::string& field_name,
                                     const boost::optional<std::string>& value)
{
    write_variable_size_field(field_name, field_kind::STRING, value);
}

void
default_compact_writer::write_array_of_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<bool>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_BOOLEAN, value);
}

void
default_compact_writer::write_array_of_int8(
  const std::string& field_name,
  const boost::optional<std::vector<int8_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT8, value);
}

void
default_compact_writer::write_array_of_int16(
  const std::string& field_name,
  const boost::optional<std::vector<int16_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT16, value);
}

void
default_compact_writer::write_array_of_int32(
  const std::string& field_name,
  const boost::optional<std::vector<int32_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT32, value);
}

void
default_compact_writer::write_array_of_int64(
  const std::string& field_name,
  const boost::optional<std::vector<int64_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT64, value);
}

void
default_compact_writer::write_array_of_float32(
  const std::string& field_name,
  const boost::optional<std::vector<float>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_FLOAT32, value);
}

void
default_compact_writer::write_array_of_float64(
  const std::string& field_name,
  const boost::optional<std::vector<double>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_FLOAT64, value);
}

void
default_compact_writer::write_array_of_string(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<std::string>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_STRING, value);
}

void
default_compact_writer::write_nullable_boolean(
  const std::string& field_name,
  const boost::optional<bool>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_BOOLEAN, value);
}
void
default_compact_writer::write_nullable_int8(
  const std::string& field_name,
  const boost::optional<int8_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT8, value);
}
void
default_compact_writer::write_nullable_int16(
  const std::string& field_name,
  const boost::optional<int16_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT16, value);
}
void
default_compact_writer::write_nullable_int32(
  const std::string& field_name,
  const boost::optional<int32_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT32, value);
}
void
default_compact_writer::write_nullable_int64(
  const std::string& field_name,
  const boost::optional<int64_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT64, value);
}
void
default_compact_writer::write_nullable_float32(
  const std::string& field_name,
  const boost::optional<float>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_FLOAT32, value);
}
void
default_compact_writer::write_nullable_float64(
  const std::string& field_name,
  const boost::optional<double>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_FLOAT64, value);
}

void
default_compact_writer::end()
{
    if (schema_.number_of_var_size_fields() == 0) {
        // There are no variable size fields
        return;
    }
    size_t position = object_data_output_.position();
    size_t data_length = position - data_start_position;
    write_offsets(data_length, field_offsets);
    // write dataLength
    object_data_output_.write_at(data_start_position -
                                   util::Bits::INT_SIZE_IN_BYTES,
                                 (int32_t)data_length);
}

size_t
default_compact_writer::get_fixed_size_field_position(
  const std::string& field_name,
  enum field_kind field_kind) const
{
    const field_descriptor& field_descriptor =
      check_field_definition(field_name, field_kind);
    return field_descriptor.offset + data_start_position;
}

const field_descriptor&
default_compact_writer::check_field_definition(const std::string& field_name,
                                               enum field_kind field_kind) const
{
    auto iterator = schema_.fields().find(field_name);
    if (iterator == schema_.fields().end()) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "default_compact_writer",
          (boost::format("Invalid field name %1% for %2%") % field_name %
           schema_)
            .str()));
    }
    if (iterator->second.field_kind != field_kind) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "default_compact_writer",
          (boost::format("Invalid field type %1% for %2%") % field_name %
           schema_)
            .str()));
    }
    return iterator->second;
}

void
default_compact_writer::write_offsets(size_t data_length,
                                      const std::vector<int32_t>& offsets)
{
    if (data_length < offset_reader::BYTE_OFFSET_READER_RANGE) {
        for (int32_t offset : offsets) {
            object_data_output_.write<int8_t>(static_cast<int8_t>(offset));
        }
    } else if (data_length < offset_reader::SHORT_OFFSET_READER_RANGE) {
        for (int32_t offset : offsets) {
            object_data_output_.write<int16_t>(static_cast<int16_t>(offset));
        }
    } else {
        for (int32_t offset : offsets) {
            object_data_output_.write<int32_t>(offset);
        }
    }
}

void
default_compact_writer::set_position(const std::string& field_name,
                                     enum field_kind field_kind)
{
    const auto& field_descriptor =
      check_field_definition(field_name, field_kind);
    size_t pos = object_data_output_.position();
    size_t fieldPosition = pos - data_start_position;
    int index = field_descriptor.index;
    field_offsets[index] = static_cast<int32_t>(fieldPosition);
}

void
default_compact_writer::set_position_as_null(const std::string& field_name,
                                             enum field_kind field_kind)
{
    const auto& field_descriptor =
      check_field_definition(field_name, field_kind);
    int index = field_descriptor.index;
    field_offsets[index] = -1;
}

namespace rabin_finger_print {
/**
 * We use uint64_t for computation to match the behaviour of >>> operator
 * on java. We use >> instead.
 */
constexpr uint64_t INIT = 0xc15d213aa4d7a795L;

static std::array<uint64_t, 256>
init_fp_table()
{
    static std::array<uint64_t, 256> FP_TABLE;
    for (int i = 0; i < 256; ++i) {
        uint64_t fp = i;
        for (int j = 0; j < 8; ++j) {
            fp = (fp >> 1) ^ (INIT & -(fp & 1L));
        }
        FP_TABLE[i] = fp;
    }
    return FP_TABLE;
}
uint64_t
FP_TABLE_AT(int index)
{
    static auto FP_TABLE = init_fp_table();
    return FP_TABLE[index];
}

uint64_t
fingerprint64(uint64_t fp, byte b)
{
    return (fp >> 8) ^ FP_TABLE_AT((int)(fp ^ b) & 0xff);
}

uint64_t
fingerprint64(uint64_t fp, int v)
{
    fp = fingerprint64(fp, (byte)((v)&0xFF));
    fp = fingerprint64(fp, (byte)((v >> 8) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 16) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 24) & 0xFF));
    return fp;
}

uint64_t
fingerprint64(uint64_t fp, const std::string& value)
{
    fp = fingerprint64(fp, (int)value.size());
    for (const auto& item : value) {
        fp = fingerprint64(fp, (byte)item);
    }
    return fp;
}

/**
 * Calculates the fingerprint of the schema from its type name and fields.
 */
int64_t
fingerprint64(const std::string& type_name,
              std::map<std::string, field_descriptor>& fields)
{
    uint64_t fingerPrint = fingerprint64(INIT, type_name);
    fingerPrint = fingerprint64(fingerPrint, (int)fields.size());
    for (const auto& entry : fields) {
        const field_descriptor& descriptor = entry.second;
        fingerPrint = fingerprint64(fingerPrint, entry.first);
        fingerPrint = fingerprint64(fingerPrint, (int)descriptor.field_kind);
    }
    return static_cast<int64_t>(fingerPrint);
}

} // namespace rabin_finger_print

bool
kind_size_comparator(const field_descriptor* i, const field_descriptor* j)
{
    auto i_kind_size =
      field_operations::get(i->field_kind).kind_size_in_byte_func();
    auto j_kind_size =
      field_operations::get(j->field_kind).kind_size_in_byte_func();
    return i_kind_size > j_kind_size;
}

schema::schema(
  std::string type_name,
  std::unordered_map<std::string, field_descriptor>&& field_definition_map)
  : type_name_(std::move(type_name))
  , field_definition_map_(std::move(field_definition_map))
{
    std::vector<field_descriptor*> fixed_size_fields;
    std::vector<field_descriptor*> boolean_fields;
    std::vector<field_descriptor*> variable_size_fields;

    std::map<std::string, field_descriptor> sorted_fields(
      field_definition_map_.begin(), field_definition_map_.end());
    for (auto& item : sorted_fields) {
        field_descriptor& descriptor = item.second;
        field_kind kind = descriptor.field_kind;
        if (field_operations::get(kind).kind_size_in_byte_func() ==
            field_kind_based_operations::VARIABLE_SIZE) {
            variable_size_fields.push_back(&descriptor);
        } else if (kind == field_kind::BOOLEAN) {
            boolean_fields.push_back(&descriptor);
        } else {
            fixed_size_fields.push_back(&descriptor);
        }
    }

    std::sort(
      fixed_size_fields.begin(), fixed_size_fields.end(), kind_size_comparator);

    int offset = 0;
    for (auto descriptor : fixed_size_fields) {
        descriptor->offset = offset;
        offset += field_operations::get(descriptor->field_kind)
                    .kind_size_in_byte_func();
    }

    int8_t bit_offset = 0;
    for (auto descriptor : boolean_fields) {
        descriptor->offset = offset;
        descriptor->bit_offset =
          static_cast<int8_t>(bit_offset % util::Bits::BITS_IN_BYTE);
        bit_offset++;
        if (bit_offset % util::Bits::BITS_IN_BYTE == 0) {
            offset += 1;
        }
    }
    if (bit_offset % util::Bits::BITS_IN_BYTE != 0) {
        offset += 1;
    }

    fixed_size_fields_length_ = offset;

    int index = 0;
    for (auto descriptor : variable_size_fields) {
        descriptor->index = index;
        index++;
    }

    for (auto& item : sorted_fields) {
        field_definition_map_[item.first] = item.second;
    }

    number_of_var_size_fields_ = index;
    schema_id_ = rabin_finger_print::fingerprint64(type_name_, sorted_fields);
}

int64_t
schema::schema_id() const
{
    return schema_id_;
}

size_t
schema::number_of_var_size_fields() const
{
    return number_of_var_size_fields_;
}

size_t
schema::fixed_size_fields_length() const
{
    return fixed_size_fields_length_;
}

const std::string&
schema::type_name() const
{
    return type_name_;
}

const std::unordered_map<std::string, field_descriptor>&
schema::fields() const
{
    return field_definition_map_;
}

std::ostream&
operator<<(std::ostream& os, const schema& schema)
{
    os << "type name " << schema.type_name() << ",number of var size fields "
       << schema.number_of_var_size_fields() << ",fixed size fields length "
       << schema.fixed_size_fields_length() << ",fields {";
    for (const auto& item : schema.fields()) {
        os << item.first << " " << item.second << ",";
    }
    os << "}";
    return os;
}

std::ostream&
operator<<(std::ostream& os, const field_descriptor& field_descriptor)
{
    os << field_descriptor.field_kind;
    return os;
}

} // namespace pimpl

namespace pimpl {
schema_writer::schema_writer(std::string type_name)
  : type_name(std::move(type_name))
{}
void
schema_writer::add_field(std::string field_name, enum field_kind kind)
{
    field_definition_map[std::move(field_name)] =
      field_descriptor{ kind, -1, -1, -1 };
}

schema
schema_writer::build() &&
{
    return schema{ type_name, std::move(field_definition_map) };
}

schema
default_schema_service::get(int64_t schemaId)
{
    auto ptr = schemas.get(schemaId);
    if (ptr == nullptr) {
        // TODO sancar throw schema_does_not_exist;
    }
    return *ptr;
}
void
default_schema_service::put(const schema& schema_v)
{
    if (schemas.contains_key(schema_v.schema_id())) {
        return;
    }
    schemas.put(schema_v.schema_id(), std::make_shared<schema>(schema_v));
}

void
compact_stream_serializer::put_to_schema_service(const schema& schema)
{
    schema_service.put(schema);
}

field_kind_based_operations::field_kind_based_operations()
  : kind_size_in_byte_func(DEFAULT_KIND_SIZE_IN_BYTES)
{}

field_kind_based_operations::field_kind_based_operations(
  std::function<int()> kind_size_in_byte_func)
  : kind_size_in_byte_func(std::move(kind_size_in_byte_func))
{}
field_kind_based_operations
field_operations::get(enum field_kind field_kind)
{
    static const field_kind_based_operations ALL[NUMBER_OF_FIELD_KINDS] = {
        field_kind_based_operations{ []() { return 0; } },
        field_kind_based_operations{},
        field_kind_based_operations{ []() { return 1; } },
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{
          []() { return util::Bits::SHORT_SIZE_IN_BYTES; } },
        field_kind_based_operations{},
        field_kind_based_operations(
          []() { return util::Bits::INT_SIZE_IN_BYTES; }),
        field_kind_based_operations{},
        field_kind_based_operations{
          []() { return util::Bits::LONG_SIZE_IN_BYTES; } },
        field_kind_based_operations{},
        field_kind_based_operations{
          []() { return util::Bits::FLOAT_SIZE_IN_BYTES; } },
        field_kind_based_operations{},
        field_kind_based_operations{
          []() { return util::Bits::DOUBLE_SIZE_IN_BYTES; } },
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{}
    };
    return ALL[field_kind];
}
} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
