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
compact_writer::write_int32(const std::string& field_name, int32_t value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, pimpl::field_kind::INT32);
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

namespace pimpl {

void
default_compact_writer::write_int32(const std::string& field_name,
                                    int32_t value)
{}
void
default_compact_writer::write_string(const std::string& field_name,
                                     const boost::optional<std::string>& value)
{}

namespace rabin_finger_print {
/**
 * We use unsigned long for computation to match the behaviour of >>> operator
 * on java. We use >> instead.
 */
constexpr unsigned long INIT = 0xc15d213aa4d7a795L;
unsigned long*
fb_table(unsigned long* FB_TABLE)
{
    for (int i = 0; i < 256; ++i) {
        unsigned long fp = i;
        for (int j = 0; j < 8; ++j) {
            fp = (fp >> 1) ^ (INIT & -(fp & 1L));
        }
        FB_TABLE[i] = fp;
    }
    return FB_TABLE;
}
unsigned long
FB_TABLE_I(int index)
{
    static unsigned long FB_TABLE[256];
    static unsigned long* FB_TABLE_PTR = fb_table(FB_TABLE);
    return FB_TABLE[index];
}

unsigned long
fingerprint64(unsigned long fp, byte b)
{
    return (fp >> 8) ^ FB_TABLE_I((int)(fp ^ b) & 0xff);
}

unsigned long
fingerprint64(unsigned long fp, int v)
{
    fp = fingerprint64(fp, (byte)((v)&0xFF));
    fp = fingerprint64(fp, (byte)((v >> 8) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 16) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 24) & 0xFF));
    return fp;
}

unsigned long
fingerprint64(unsigned long fp, const std::string& value)
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
long
fingerprint64(const schema& schema)
{
    unsigned long fingerPrint = fingerprint64(INIT, schema.type_name());
    fingerPrint = fingerprint64(fingerPrint, (int)schema.field_count());
    for (const auto& entry : schema.fields()) {
        const field_descriptor& descriptor = entry.second;
        fingerPrint = fingerprint64(fingerPrint, descriptor.field_name());
        fingerPrint = fingerprint64(fingerPrint, (int)descriptor.field_kind());
    }
    return static_cast<long>(fingerPrint);
}

} // namespace rabin_finger_print

bool
kind_size_comparator(const field_descriptor* i, const field_descriptor* j)
{
    auto i_kind_size =
      field_operations::get(i->field_kind()).kind_size_in_byte_func();
    auto j_kind_size =
      field_operations::get(j->field_kind()).kind_size_in_byte_func();
    return i_kind_size < j_kind_size;
}

schema::schema(std::string type_name,
               std::map<std::string, field_descriptor>&& field_definition_map)
  : type_name_(std::move(type_name))
  , field_definition_map_(field_definition_map)
{
    std::vector<field_descriptor*> fixed_size_fields;
    std::vector<field_descriptor*> boolean_fields;
    std::vector<field_descriptor*> variable_size_fields;

    for (auto& item : field_definition_map_) {
        field_descriptor& descriptor = item.second;
        field_kind kind = descriptor.field_kind();
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
        descriptor->offset(offset);
        offset += field_operations::get(descriptor->field_kind())
                    .kind_size_in_byte_func();
    }

    int bit_offset = 0;
    for (auto descriptor : boolean_fields) {
        descriptor->offset(offset);
        descriptor->bit_offset(bit_offset % util::Bits::BITS_IN_BYTE);
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
        descriptor->index(index);
        index++;
    }

    number_of_var_size_fields_ = index;
    schema_id_ = rabin_finger_print::fingerprint64(*this);
}

long
schema::schema_id() const
{
    return schema_id_;
}

const field_descriptor&
schema::get_field(const std::string& field_name) const
{
    return field_definition_map_.at(field_name);
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

size_t
schema::field_count() const
{
    return field_definition_map_.size();
}

const std::map<std::string, field_descriptor>&
schema::fields() const
{
    return field_definition_map_;
}
} // namespace pimpl
int32_t
hz_serializer<pimpl::schema>::get_factory_id()
{
    return pimpl::SCHEMA_DS_FACTORY_ID;
}

int32_t
hz_serializer<pimpl::schema>::get_class_id()
{
    return pimpl::SCHEMA_CLASS_ID;
}

void
hz_serializer<pimpl::schema>::write_data(const pimpl::schema& object,
                                         object_data_output& out)
{
    out.write(object.type_name());
    const std::map<std::string, pimpl::field_descriptor>& fields =
      object.fields();
    out.write((int)fields.size());
    for (const auto& field : fields) {
        const auto& descriptor = field.second;
        out.write(descriptor.field_name());
        out.write((int)descriptor.field_kind());
    }
}

pimpl::schema
hz_serializer<pimpl::schema>::read_data(object_data_input& in)
{
    auto type_name = in.read<std::string>();
    int field_definitions_size = in.read<int>();

    std::map<std::string, pimpl::field_descriptor> field_definition_map;
    for (int i = 0; i < field_definitions_size; ++i) {
        auto field_name = in.read<std::string>();
        auto field_kind = (pimpl::field_kind)in.read<int>();
        // TODO sancar is it safe to emplace field_name and use it again
        field_definition_map.emplace(
          field_name, pimpl::field_descriptor{ field_name, field_kind });
    }
    return pimpl::schema{ type_name, std::move(field_definition_map) };
}

namespace pimpl {
schema_writer::schema_writer(std::string type_name)
  : type_name(std::move(type_name))
{}
void
schema_writer::add_field(const std::string& field_name, enum field_kind kind)
{
    field_definition_map.emplace(field_name,
                                 field_descriptor{ field_name, kind });
}

schema
schema_writer::build() &&
{
    return schema{ type_name, std::move(field_definition_map) };
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
default_schema_service::put(const std::shared_ptr<schema>& schema_ptr)
{
    schemas.put(schema_ptr->schema_id(), schema_ptr);
    return boost::make_ready_future();
}

field_kind_based_operations::field_kind_based_operations()
  : kind_size_in_byte_func(DEFAULT_KIND_SIZE_IN_BYTES)
{}

field_kind_based_operations::field_kind_based_operations(
  std::function<int()> kind_size_in_byte_func)
  : kind_size_in_byte_func(std::move(kind_size_in_byte_func))
{}

field_kind_based_operations
field_operations::get(int index)
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
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{},
        field_kind_based_operations{}
    };
    return ALL[index];
}
} // namespace pimpl

} // namespace serialization
} // namespace client
} // namespace hazelcast
