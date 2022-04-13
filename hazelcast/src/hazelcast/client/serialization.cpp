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

#include <boost/concept_check.hpp>
#include <utility>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/hazelcast_json_value.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/client_config.h"

namespace hazelcast {
namespace client {
hazelcast_json_value::hazelcast_json_value(std::string json_string)
  : json_string_(std::move(json_string))
{}

hazelcast_json_value::~hazelcast_json_value() = default;

const std::string&
hazelcast_json_value::to_string() const
{
    return json_string_;
}

bool
hazelcast_json_value::operator==(const hazelcast_json_value& rhs) const
{
    return json_string_ == rhs.json_string_;
}

bool
hazelcast_json_value::operator!=(const hazelcast_json_value& rhs) const
{
    return !(rhs == *this);
}

bool
hazelcast_json_value::operator<(const hazelcast_json_value& rhs) const
{
    return json_string_ < rhs.json_string_;
}

std::ostream&
operator<<(std::ostream& os, const hazelcast_json_value& value)
{
    os << "jsonString: " << value.json_string_;
    return os;
}

typed_data::typed_data()
  : ss_(nullptr)
{}

typed_data::typed_data(
  serialization::pimpl::data d,
  serialization::pimpl::SerializationService& serialization_service)
  : data_(std::move(d))
  , ss_(&serialization_service)
{}

serialization::pimpl::object_type
typed_data::get_type() const
{
    return ss_->get_object_type(&data_);
}

const serialization::pimpl::data&
typed_data::get_data() const
{
    return data_;
}

bool
operator<(const typed_data& lhs, const typed_data& rhs)
{
    const auto& lhs_data = lhs.get_data();
    const auto& rhs_data = rhs.get_data();

    return lhs_data < rhs_data;
}

namespace serialization {
portable_writer::portable_writer(
  pimpl::DefaultPortableWriter* default_portable_writer)
  : default_portable_writer_(default_portable_writer)
  , class_definition_writer_(nullptr)
  , is_default_writer_(true)
{}

portable_writer::portable_writer(
  pimpl::ClassDefinitionWriter* class_definition_writer)
  : default_portable_writer_(nullptr)
  , class_definition_writer_(class_definition_writer)
  , is_default_writer_(false)
{}

void
portable_writer::end()
{
    if (is_default_writer_)
        return default_portable_writer_->end();
    return class_definition_writer_->end();
}

object_data_output&
portable_writer::get_raw_data_output()
{
    if (is_default_writer_)
        return default_portable_writer_->get_raw_data_output();
    return class_definition_writer_->get_raw_data_output();
}

ClassDefinitionBuilder::ClassDefinitionBuilder(int factory_id,
                                               int class_id,
                                               int version)
  : factory_id_(factory_id)
  , class_id_(class_id)
  , version_(version)
  , index_(0)
  , done_(false)
{}

ClassDefinitionBuilder&
ClassDefinitionBuilder::add_portable_field(const std::string& field_name,
                                           std::shared_ptr<ClassDefinition> def)
{
    check();
    if (def->get_class_id() == 0) {
        BOOST_THROW_EXCEPTION(exception::illegal_argument(
          "ClassDefinitionBuilder::addPortableField",
          "Portable class id cannot be zero!"));
    }
    FieldDefinition fieldDefinition(index_++,
                                    field_name,
                                    field_type::TYPE_PORTABLE,
                                    def->get_factory_id(),
                                    def->get_class_id(),
                                    def->get_version());
    field_definitions_.push_back(fieldDefinition);
    return *this;
}

ClassDefinitionBuilder&
ClassDefinitionBuilder::add_portable_array_field(
  const std::string& field_name,
  std::shared_ptr<ClassDefinition> def)
{
    check();
    if (def->get_class_id() == 0) {
        BOOST_THROW_EXCEPTION(exception::illegal_argument(
          "ClassDefinitionBuilder::addPortableField",
          "Portable class id cannot be zero!"));
    }
    FieldDefinition fieldDefinition(index_++,
                                    field_name,
                                    field_type::TYPE_PORTABLE_ARRAY,
                                    def->get_factory_id(),
                                    def->get_class_id(),
                                    def->get_version());
    field_definitions_.push_back(fieldDefinition);
    return *this;
}

ClassDefinitionBuilder&
ClassDefinitionBuilder::add_field(FieldDefinition& field_definition)
{
    check();
    int defIndex = field_definition.get_index();
    if (index_ != defIndex) {
        char buf[100];
        util::hz_snprintf(buf,
                          100,
                          "Invalid field index. Index in definition:%d, being "
                          "added at index:%d",
                          defIndex,
                          index_);
        BOOST_THROW_EXCEPTION(
          exception::illegal_argument("ClassDefinitionBuilder::addField", buf));
    }
    index_++;
    field_definitions_.push_back(field_definition);
    return *this;
}

std::shared_ptr<ClassDefinition>
ClassDefinitionBuilder::build()
{
    done_ = true;
    std::shared_ptr<ClassDefinition> cd(
      new ClassDefinition(factory_id_, class_id_, version_));

    std::vector<FieldDefinition>::iterator fdIt;
    for (fdIt = field_definitions_.begin(); fdIt != field_definitions_.end();
         fdIt++) {
        cd->add_field_def(*fdIt);
    }
    return cd;
}

void
ClassDefinitionBuilder::check()
{
    if (done_) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "ClassDefinitionBuilder::check",
          "ClassDefinition is already built for " +
            util::IOUtil::to_string(class_id_)));
    }
}

void
ClassDefinitionBuilder::add_field(const std::string& field_name,
                                  field_type const& field_type)
{
    check();
    FieldDefinition fieldDefinition(index_++, field_name, field_type, version_);
    field_definitions_.push_back(fieldDefinition);
}

int
ClassDefinitionBuilder::get_factory_id()
{
    return factory_id_;
}

int
ClassDefinitionBuilder::get_class_id()
{
    return class_id_;
}

int
ClassDefinitionBuilder::get_version()
{
    return version_;
}

FieldDefinition::FieldDefinition()
  : index_(0)
  , class_id_(0)
  , factory_id_(0)
  , version_(-1)
{}

FieldDefinition::FieldDefinition(int index,
                                 const std::string& field_name,
                                 field_type const& type,
                                 int version)
  : index_(index)
  , field_name_(field_name)
  , type_(type)
  , class_id_(0)
  , factory_id_(0)
  , version_(version)
{}

FieldDefinition::FieldDefinition(int index,
                                 const std::string& field_name,
                                 field_type const& type,
                                 int factory_id,
                                 int class_id,
                                 int version)
  : index_(index)
  , field_name_(field_name)
  , type_(type)
  , class_id_(class_id)
  , factory_id_(factory_id)
  , version_(version)
{}

const field_type&
FieldDefinition::get_type() const
{
    return type_;
}

std::string
FieldDefinition::get_name() const
{
    return field_name_;
}

int
FieldDefinition::get_index() const
{
    return index_;
}

int
FieldDefinition::get_factory_id() const
{
    return factory_id_;
}

int
FieldDefinition::get_class_id() const
{
    return class_id_;
}

void
FieldDefinition::write_data(pimpl::data_output& data_output)
{
    data_output.write<int32_t>(index_);
    data_output.write<std::string>(field_name_);
    data_output.write<byte>(static_cast<int32_t>(type_));
    data_output.write<int32_t>(factory_id_);
    data_output.write<int32_t>(class_id_);
}

void
FieldDefinition::read_data(object_data_input& data_input)
{
    index_ = data_input.read<int32_t>();
    field_name_ = data_input.read<std::string>();
    type_ = static_cast<field_type>(data_input.read<byte>());
    factory_id_ = data_input.read<int32_t>();
    class_id_ = data_input.read<int32_t>();
}

bool
FieldDefinition::operator==(const FieldDefinition& rhs) const
{
    return field_name_ == rhs.field_name_ && type_ == rhs.type_ &&
           class_id_ == rhs.class_id_ && factory_id_ == rhs.factory_id_ &&
           version_ == rhs.version_;
}

bool
FieldDefinition::operator!=(const FieldDefinition& rhs) const
{
    return !(rhs == *this);
}

std::ostream&
operator<<(std::ostream& os, const FieldDefinition& definition)
{
    os << "FieldDefinition{"
       << "index: " << definition.index_
       << " fieldName: " << definition.field_name_
       << " type: " << static_cast<int32_t>(definition.type_)
       << " classId: " << definition.class_id_
       << " factoryId: " << definition.factory_id_
       << " version: " << definition.version_;
    return os;
}

object_data_input::object_data_input(
  boost::endian::order byte_order,
  const std::vector<byte>& buffer,
  int offset,
  pimpl::PortableSerializer& portable_ser,
  pimpl::compact_stream_serializer& compact_ser,
  pimpl::DataSerializer& data_ser,
  std::shared_ptr<serialization::global_serializer> global_serializer)
  : pimpl::data_input<std::vector<byte>>(byte_order, buffer, offset)
  , portable_serializer_(portable_ser)
  , compact_serializer_(compact_ser)
  , data_serializer_(data_ser)
  , global_serializer_(std::move(global_serializer))
{}

object_data_output::object_data_output(
  boost::endian::order byte_order,
  bool dont_write,
  pimpl::PortableSerializer* portable_ser,
  pimpl::compact_stream_serializer* compact_ser,
  std::shared_ptr<serialization::global_serializer> global_serializer)
  : data_output(byte_order, dont_write)
  , portable_serializer_(portable_ser)
  , compact_serializer_(compact_ser)
  , global_serializer_(std::move(global_serializer))
{}

portable_reader::portable_reader(pimpl::PortableSerializer& portable_ser,
                                 object_data_input& input,
                                 const std::shared_ptr<ClassDefinition>& cd,
                                 bool is_default_reader)
  : is_default_reader_(is_default_reader)
{
    if (is_default_reader) {
        default_portable_reader_ = boost::make_optional(
          pimpl::DefaultPortableReader(portable_ser, input, cd));
    } else {
        morphing_portable_reader_ = boost::make_optional(
          pimpl::MorphingPortableReader(portable_ser, input, cd));
    }
}

object_data_input&
portable_reader::get_raw_data_input()
{
    if (is_default_reader_)
        return default_portable_reader_->get_raw_data_input();
    return morphing_portable_reader_->get_raw_data_input();
}

template<>
void
object_data_output::write_object(const char* object)
{
    if (!object) {
        write<int32_t>(static_cast<int32_t>(
          pimpl::serialization_constants::CONSTANT_TYPE_NULL));
        return;
    }
    write_object<std::string>(std::string(object));
}

void
portable_reader::end()
{
    if (is_default_reader_)
        return default_portable_reader_->end();
    return morphing_portable_reader_->end();
}

ClassDefinition::ClassDefinition()
  : factory_id_(0)
  , class_id_(0)
  , version_(-1)
  , binary_(new std::vector<byte>)
{}

ClassDefinition::ClassDefinition(int factory_id, int class_id, int version)
  : factory_id_(factory_id)
  , class_id_(class_id)
  , version_(version)
  , binary_(new std::vector<byte>)
{}

void
ClassDefinition::add_field_def(FieldDefinition& fd)
{
    field_definitions_map_[fd.get_name()] = fd;
}

const FieldDefinition&
ClassDefinition::get_field(const std::string& name) const
{
    auto it = field_definitions_map_.find(name);
    if (it != field_definitions_map_.end()) {
        return it->second;
    }
    BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
      "ClassDefinition::getField",
      (boost::format("Invalid field name: '%1%' for ClassDefinition {id: %2%, "
                     "version: %3%}") %
       name % class_id_ % version_)
        .str()));
}

bool
ClassDefinition::has_field(const std::string& field_name) const
{
    return field_definitions_map_.find(field_name) !=
           field_definitions_map_.end();
}

field_type
ClassDefinition::get_field_type(const std::string& field_name) const
{
    FieldDefinition const& fd = get_field(field_name);
    return fd.get_type();
}

int
ClassDefinition::get_field_count() const
{
    return (int)field_definitions_map_.size();
}

int
ClassDefinition::get_factory_id() const
{
    return factory_id_;
}

int
ClassDefinition::get_class_id() const
{
    return class_id_;
}

int
ClassDefinition::get_version() const
{
    return version_;
}

void
ClassDefinition::set_version_if_not_set(int new_version)
{
    if (get_version() < 0) {
        this->version_ = new_version;
    }
}

void
ClassDefinition::write_data(pimpl::data_output& data_output)
{
    data_output.write<int32_t>(factory_id_);
    data_output.write<int32_t>(class_id_);
    data_output.write<int32_t>(version_);
    data_output.write<int16_t>(field_definitions_map_.size());
    for (auto& entry : field_definitions_map_) {
        entry.second.write_data(data_output);
    }
}

void
ClassDefinition::read_data(object_data_input& data_input)
{
    factory_id_ = data_input.read<int32_t>();
    class_id_ = data_input.read<int32_t>();
    version_ = data_input.read<int32_t>();
    int size = data_input.read<int16_t>();
    for (int i = 0; i < size; i++) {
        FieldDefinition fieldDefinition;
        fieldDefinition.read_data(data_input);
        add_field_def(fieldDefinition);
    }
}

bool
ClassDefinition::operator==(const ClassDefinition& rhs) const
{
    return factory_id_ == rhs.factory_id_ && class_id_ == rhs.class_id_ &&
           version_ == rhs.version_ &&
           field_definitions_map_ == rhs.field_definitions_map_;
}

bool
ClassDefinition::operator!=(const ClassDefinition& rhs) const
{
    return !(rhs == *this);
}

std::ostream&
operator<<(std::ostream& os, const ClassDefinition& definition)
{
    os << "ClassDefinition{"
       << "factoryId: " << definition.factory_id_
       << " classId: " << definition.class_id_
       << " version: " << definition.version_ << " fieldDefinitions: {";

    for (auto& entry : definition.field_definitions_map_) {
        os << entry.second;
    }
    os << "} }";
    return os;
}

namespace pimpl {
ClassDefinitionWriter::ClassDefinitionWriter(PortableContext& portable_context,
                                             ClassDefinitionBuilder& builder)
  : builder_(builder)
  , context_(portable_context)
  , empty_data_output_(
      portable_context.get_serialization_config().get_byte_order(),
      true)
{}

std::shared_ptr<ClassDefinition>
ClassDefinitionWriter::register_and_get()
{
    std::shared_ptr<ClassDefinition> cd = builder_.build();
    return context_.register_class_definition(cd);
}

object_data_output&
ClassDefinitionWriter::get_raw_data_output()
{
    return empty_data_output_;
}

void
ClassDefinitionWriter::end()
{}

data_output::data_output(boost::endian::order byte_order, bool dont_write)
  : byte_order_(byte_order)
  , is_no_write_(dont_write)
{
    if (is_no_write_) {
        output_stream_.reserve(0);
    } else {
        output_stream_.reserve(DEFAULT_SIZE);
    }
}

template<>
void
data_output::write(byte i)
{
    if (is_no_write_) {
        return;
    }
    output_stream_.push_back(i);
}

template<>
void
data_output::write(char i)
{
    // C++ `char` is one byte only, `char16_t` is two bytes
    write<int16_t>(i);
}

template<>
void
data_output::write(char16_t i)
{
    write<int16_t>(i);
}

template<>
void
data_output::write(int16_t value)
{
    if (is_no_write_) {
        return;
    }
    if (byte_order_ == boost::endian::order::big) {
        boost::endian::native_to_big_inplace(value);
    } else {
        boost::endian::native_to_little_inplace(value);
    }
    output_stream_.insert(output_stream_.end(),
                          (byte*)&value,
                          (byte*)&value + util::Bits::SHORT_SIZE_IN_BYTES);
}

void
data_output::write(int32_t value, boost::endian::order byte_order)
{
    if (is_no_write_) {
        return;
    }
    if (byte_order == boost::endian::order::big) {
        boost::endian::native_to_big_inplace(value);
    } else {
        boost::endian::native_to_little_inplace(value);
    }
    output_stream_.insert(output_stream_.end(),
                          (byte*)&value,
                          (byte*)&value + util::Bits::INT_SIZE_IN_BYTES);
}

template<>
void
data_output::write(int32_t value)
{
    write(value, byte_order_);
}

template<>
void
data_output::write(int64_t value)
{
    if (is_no_write_) {
        return;
    }
    if (byte_order_ == boost::endian::order::big) {
        boost::endian::native_to_big_inplace(value);
    } else {
        boost::endian::native_to_little_inplace(value);
    }
    output_stream_.insert(output_stream_.end(),
                          (byte*)&value,
                          (byte*)&value + util::Bits::LONG_SIZE_IN_BYTES);
}

template<>
void
data_output::write(float x)
{
    if (is_no_write_) {
        return;
    }
    union
    {
        float f;
        int32_t i;
    } u;
    u.f = x;
    write<int32_t>(u.i);
}

template<>
void
data_output::write(double v)
{
    if (is_no_write_) {
        return;
    }
    union
    {
        double d;
        int64_t l;
    } u;
    u.d = v;
    write<int64_t>(u.l);
}

template<>
void
data_output::write(boost::uuids::uuid v)
{
    if (is_no_write_) {
        return;
    }
    if (byte_order_ == boost::endian::order::little) {
        boost::endian::endian_reverse_inplace<int64_t>(
          *reinterpret_cast<int64_t*>(v.data));
        boost::endian::endian_reverse_inplace<int64_t>(
          *reinterpret_cast<int64_t*>(&v.data[util::Bits::LONG_SIZE_IN_BYTES]));
    }
    output_stream_.insert(
      output_stream_.end(), v.data, v.data + util::Bits::UUID_SIZE_IN_BYTES);
}

template<>
void
data_output::write(bool value)
{
    if (is_no_write_) {
        return;
    }
    write<byte>(value);
}

template<>
void
data_output::write(const std::string& str)
{
    if (is_no_write_) {
        return;
    }

    write<int32_t>(str.size());
    output_stream_.insert(output_stream_.end(), str.begin(), str.end());
}

template<>
void
data_output::write(const hazelcast_json_value& value)
{
    if (is_no_write_) {
        return;
    }
    write<std::string>(value.to_string());
}

object_type::object_type()
  : type_id(serialization_constants::CONSTANT_TYPE_NULL)
  , factory_id(-1)
  , class_id(-1)
{}

std::ostream&
operator<<(std::ostream& os, const object_type& type)
{
    os << "typeId: " << static_cast<int32_t>(type.type_id)
       << " factoryId: " << type.factory_id << " classId: " << type.class_id;
    return os;
}

int32_t
DataSerializer::read_int(object_data_input& in) const
{
    return in.read<int32_t>();
}

PortableContext::PortableContext(const serialization_config& serialization_conf)
  : serialization_config_(serialization_conf)
{}

int
PortableContext::get_class_version(int factory_id, int class_id)
{
    return get_class_definition_context(factory_id).get_class_version(class_id);
}

void
PortableContext::set_class_version(int factory_id, int class_id, int version)
{
    get_class_definition_context(factory_id)
      .set_class_version(class_id, version);
}

std::shared_ptr<ClassDefinition>
PortableContext::lookup_class_definition(int factory_id,
                                         int class_id,
                                         int version)
{
    return get_class_definition_context(factory_id).lookup(class_id, version);
}

std::shared_ptr<ClassDefinition>
PortableContext::read_class_definition(object_data_input& in,
                                       int factory_id,
                                       int class_id,
                                       int version)
{
    bool shouldRegister = true;
    ClassDefinitionBuilder builder(factory_id, class_id, version);

    // final position after portable is read
    in.read<int32_t>();

    // field count
    int fieldCount = in.read<int32_t>();
    int offset = in.position();
    for (int i = 0; i < fieldCount; i++) {
        in.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
        int pos = in.read<int32_t>();
        in.position(pos);

        short len = in.read<int16_t>();
        std::vector<byte> chars(len);
        in.read_fully(chars);
        chars.push_back('\0');

        field_type type(static_cast<field_type>(in.read<byte>()));
        std::string name((char*)&(chars[0]));
        int fieldFactoryId = 0;
        int fieldClassId = 0;
        int fieldVersion = version;
        if (type == field_type::TYPE_PORTABLE) {
            // is null
            if (in.read<bool>()) {
                shouldRegister = false;
            }
            fieldFactoryId = in.read<int32_t>();
            fieldClassId = in.read<int32_t>();

            // TODO: what if there's a null inner Portable field
            if (shouldRegister) {
                fieldVersion = in.read<int32_t>();
                read_class_definition(
                  in, fieldFactoryId, fieldClassId, fieldVersion);
            }
        } else if (type == field_type::TYPE_PORTABLE_ARRAY) {
            int k = in.read<int32_t>();
            if (k > 0) {
                fieldFactoryId = in.read<int32_t>();
                fieldClassId = in.read<int32_t>();

                int p = in.read<int32_t>();
                in.position(p);

                // TODO: what if there's a null inner Portable field
                fieldVersion = in.read<int32_t>();
                read_class_definition(
                  in, fieldFactoryId, fieldClassId, fieldVersion);
            } else {
                shouldRegister = false;
            }
        }
        FieldDefinition fieldDef(
          i, name, type, fieldFactoryId, fieldClassId, fieldVersion);
        builder.add_field(fieldDef);
    }
    std::shared_ptr<ClassDefinition> classDefinition = builder.build();
    if (shouldRegister) {
        classDefinition = register_class_definition(classDefinition);
    }
    return classDefinition;
}

std::shared_ptr<ClassDefinition>
PortableContext::register_class_definition(std::shared_ptr<ClassDefinition> cd)
{
    return get_class_definition_context(cd->get_factory_id())
      .register_class_definition(cd);
}

int
PortableContext::get_version()
{
    return serialization_config_.get_portable_version();
}

ClassDefinitionContext&
PortableContext::get_class_definition_context(int factory_id)
{
    std::shared_ptr<ClassDefinitionContext> value =
      class_def_context_map_.get(factory_id);
    if (value == NULL) {
        value = std::shared_ptr<ClassDefinitionContext>(
          new ClassDefinitionContext(factory_id, this));
        std::shared_ptr<ClassDefinitionContext> current =
          class_def_context_map_.put_if_absent(factory_id, value);
        if (current != NULL) {
            value = current;
        }
    }
    return *value;
}

const serialization_config&
PortableContext::get_serialization_config() const
{
    return serialization_config_;
}

SerializationService::SerializationService(const serialization_config& config)
  : serialization_config_(config)
  , portable_context_(serialization_config_)
  , portable_serializer_(portable_context_)
  , compact_serializer_()
{}

DefaultPortableWriter::DefaultPortableWriter(
  PortableSerializer& portable_ser,
  std::shared_ptr<ClassDefinition> cd,
  object_data_output& output)
  : raw_(false)
  , portable_serializer_(portable_ser)
  , object_data_output_(output)
  , begin_(object_data_output_.position())
  , cd_(cd)
{
    // room for final offset
    object_data_output_.write<int32_t>(0);

    object_data_output_.write<int32_t>(cd->get_field_count());

    offset_ = object_data_output_.position();
    // one additional for raw data
    int fieldIndexesLength =
      (cd->get_field_count() + 1) * util::Bits::INT_SIZE_IN_BYTES;
    object_data_output_.write_zero_bytes(fieldIndexesLength);
}

FieldDefinition const&
DefaultPortableWriter::set_position(const std::string& field_name,
                                    field_type field_type)
{
    if (raw_) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "PortableWriter::setPosition",
          "Cannot write Portable fields after getRawDataOutput() is called!"));
    }

    try {
        FieldDefinition const& fd = cd_->get_field(field_name);

        if (written_fields_.find(field_name) != written_fields_.end()) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
              "PortableWriter::setPosition",
              "Field '" + std::string(field_name) +
                "' has already been written!"));
        }

        written_fields_.insert(field_name);
        size_t pos = object_data_output_.position();
        int32_t index = fd.get_index();
        object_data_output_.write_at(offset_ +
                                       index * util::Bits::INT_SIZE_IN_BYTES,
                                     static_cast<int32_t>(pos));
        object_data_output_.write(static_cast<int16_t>(field_name.size()));
        object_data_output_.write_bytes(field_name);
        object_data_output_.write<byte>(static_cast<byte>(field_type));

        return fd;

    } catch (exception::illegal_argument& iae) {
        std::stringstream error;
        error << "hazelcast_serialization_exception( Invalid field name: '"
              << field_name;
        error << "' for ClassDefinition {class id: "
              << util::IOUtil::to_string(cd_->get_class_id());
        error << ", factoryId:" +
                   util::IOUtil::to_string(cd_->get_factory_id());
        error << ", version: " << util::IOUtil::to_string(cd_->get_version())
              << "}. Error:";
        error << iae.what();

        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "PortableWriter::setPosition", error.str()));
    }
}

object_data_output&
DefaultPortableWriter::get_raw_data_output()
{
    if (!raw_) {
        size_t pos = object_data_output_.position();
        int32_t index = cd_->get_field_count(); // last index
        object_data_output_.write_at(offset_ +
                                       index * util::Bits::INT_SIZE_IN_BYTES,
                                     static_cast<int32_t>(pos));
    }
    raw_ = true;
    return object_data_output_;
}

void
DefaultPortableWriter::end()
{
    object_data_output_.write_at(
      begin_, static_cast<int32_t>(object_data_output_.position()));
}

bool
SerializationService::is_null_data(const data& data)
{
    return data.data_size() == 0 &&
           data.get_type() ==
             static_cast<int32_t>(serialization_constants::CONSTANT_TYPE_NULL);
}

template<>
data
SerializationService::to_data(const char* object)
{
    if (!object) {
        return to_data<std::string>(nullptr);
    }
    std::string str(object);
    return to_data<std::string>(str);
}

const byte
SerializationService::get_version() const
{
    return 1;
}

object_type
SerializationService::get_object_type(const data* data)
{
    object_type type;

    if (NULL == data) {
        type.type_id = serialization_constants::CONSTANT_TYPE_NULL;
        return type;
    }

    type.type_id = static_cast<serialization_constants>(data->get_type());

    if (serialization_constants::CONSTANT_TYPE_DATA == type.type_id ||
        serialization_constants::CONSTANT_TYPE_PORTABLE == type.type_id) {
        // 8 (data Header) = Hash(4-bytes) + data TypeId(4 bytes)
        data_input<std::vector<byte>> dataInput(
          serialization_config_.get_byte_order(), data->to_byte_array(), 8);

        if (serialization_constants::CONSTANT_TYPE_DATA == type.type_id) {
            bool identified = dataInput.read<bool>();
            if (!identified) {
                BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
                  "SerializationService::getObjectType",
                  " DataSerializable is not 'identified data'"));
            }
        }

        type.factory_id = dataInput.read<int32_t>();
        type.class_id = dataInput.read<int32_t>();
    }

    return type;
}

void
SerializationService::dispose()
{}

PortableSerializer&
SerializationService::get_portable_serializer()
{
    return portable_serializer_;
}

compact_stream_serializer&
SerializationService::get_compact_serializer()
{
    return compact_serializer_;
}

DataSerializer&
SerializationService::get_data_serializer()
{
    return data_serializer_;
}

object_data_output
SerializationService::new_output_stream()
{
    return object_data_output(serialization_config_.get_byte_order(),
                              false,
                              &portable_serializer_,
                              &compact_serializer_,
                              serialization_config_.get_global_serializer());
}

template<>
data
SerializationService::to_data(const typed_data* object)
{
    if (!object) {
        return data();
    }

    return object->get_data();
}

// first 4 byte is partition hash code and next last 4 byte is type id
unsigned int data::PARTITION_HASH_OFFSET = 0;

unsigned int data::TYPE_OFFSET =
  data::PARTITION_HASH_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

unsigned int data::DATA_OFFSET =
  data::TYPE_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

unsigned int data::DATA_OVERHEAD = data::DATA_OFFSET;

data::data()
  : cached_hash_value_(-1)
{}

data::data(std::vector<byte> buffer)
  : data_(std::move(buffer))
  , cached_hash_value_(-1)
{
    size_t size = data_.size();
    if (size > 0 && size < data::DATA_OVERHEAD) {
        throw(exception::exception_builder<exception::illegal_argument>(
                "Data::setBuffer")
              << "Provided buffer should be either empty or should contain "
                 "more than "
              << data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size)
          .build();
    }

    cached_hash_value_ = calculate_hash();
}

size_t
data::data_size() const
{
    return (size_t)std::max<int>((int)total_size() - (int)data::DATA_OVERHEAD,
                                 0);
}

size_t
data::total_size() const
{
    return data_.size();
}

int
data::get_partition_hash() const
{
    return cached_hash_value_;
}

bool
data::has_partition_hash() const
{
    return data_.size() >= data::DATA_OVERHEAD &&
           *reinterpret_cast<const int32_t*>(&data_[PARTITION_HASH_OFFSET]) !=
             0;
}

const std::vector<byte>&
data::to_byte_array() const
{
    return data_;
}

int32_t
data::get_type() const
{
    if (total_size() == 0) {
        return static_cast<int32_t>(
          serialization_constants::CONSTANT_TYPE_NULL);
    }
    return boost::endian::
      endian_load<boost::uint32_t, 4, boost::endian::order::big>(
        &data_[data::TYPE_OFFSET]);
}

int
data::hash() const
{
    return cached_hash_value_;
}

int
data::calculate_hash() const
{
    size_t size = data_size();
    if (size == 0) {
        return 0;
    }

    if (has_partition_hash()) {
        return boost::endian::
          endian_load<boost::uint32_t, 4, boost::endian::order::big>(
            &data_[data::PARTITION_HASH_OFFSET]);
    }

    return util::murmur_hash3_x86_32((void*)&((data_)[data::DATA_OFFSET]),
                                     (int)size);
}

bool
data::operator<(const data& rhs) const
{
    return cached_hash_value_ < rhs.cached_hash_value_;
}

bool
operator==(const data& lhs, const data& rhs)
{
    return lhs.data_ == rhs.data_;
}

ClassDefinitionContext::ClassDefinitionContext(
  int factory_id,
  PortableContext* portable_context)
  : factory_id_(factory_id)
  , portable_context_(portable_context)
{}

int
ClassDefinitionContext::get_class_version(int class_id)
{
    std::shared_ptr<int> version = current_class_versions_.get(class_id);
    return version != NULL ? *version : -1;
}

void
ClassDefinitionContext::set_class_version(int class_id, int version)
{
    std::shared_ptr<int> current = current_class_versions_.put_if_absent(
      class_id, std::shared_ptr<int>(new int(version)));
    if (current != NULL && *current != version) {
        std::stringstream error;
        error << "Class-id: " << class_id << " is already registered!";
        BOOST_THROW_EXCEPTION(exception::illegal_argument(
          "ClassDefinitionContext::setClassVersion", error.str()));
    }
}

std::shared_ptr<ClassDefinition>
ClassDefinitionContext::lookup(int class_id, int version)
{
    long long key = combine_to_long(class_id, version);
    return versioned_definitions_.get(key);
}

std::shared_ptr<ClassDefinition>
ClassDefinitionContext::register_class_definition(
  std::shared_ptr<ClassDefinition> cd)
{
    if (cd.get() == NULL) {
        return std::shared_ptr<ClassDefinition>();
    }
    if (cd->get_factory_id() != factory_id_) {
        throw(exception::exception_builder<exception::hazelcast_serialization>(
                "ClassDefinitionContext::registerClassDefinition")
              << "Invalid factory-id! " << factory_id_ << " -> " << cd)
          .build();
    }

    cd->set_version_if_not_set(portable_context_->get_version());

    long long versionedClassId =
      combine_to_long(cd->get_class_id(), cd->get_version());
    std::shared_ptr<ClassDefinition> currentCd =
      versioned_definitions_.put_if_absent(versionedClassId, cd);
    if (currentCd.get() == NULL) {
        return cd;
    }

    if (currentCd.get() != cd.get() && *currentCd != *cd) {
        throw(exception::exception_builder<exception::hazelcast_serialization>(
                "ClassDefinitionContext::registerClassDefinition")
              << "Incompatible class-definitions with same class-id: " << *cd
              << " VS " << *currentCd)
          .build();
    }

    return currentCd;
}

int64_t
ClassDefinitionContext::combine_to_long(int x, int y) const
{
    return ((int64_t)x) << 32 | (((int64_t)y) & 0xFFFFFFFL);
}

DefaultPortableReader::DefaultPortableReader(
  PortableSerializer& portable_ser,
  object_data_input& input,
  std::shared_ptr<ClassDefinition> cd)
  : PortableReaderBase(portable_ser, input, cd)
{}

PortableReaderBase::PortableReaderBase(PortableSerializer& portable_ser,
                                       object_data_input& input,
                                       std::shared_ptr<ClassDefinition> cd)
  : cd_(cd)
  , data_input_(&input)
  , portable_serializer_(&portable_ser)
  , raw_(false)
{
    int fieldCount;
    try {
        // final position after portable is read
        final_position_ = input.read<int32_t>();
        // field count
        fieldCount = input.read<int32_t>();
    } catch (exception::iexception& e) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "[PortableReaderBase::PortableReaderBase]", e.what()));
    }
    if (fieldCount != cd->get_field_count()) {
        char msg[50];
        util::hz_snprintf(msg,
                          50,
                          "Field count[%d] in stream does not match %d",
                          fieldCount,
                          cd->get_field_count());
        BOOST_THROW_EXCEPTION(exception::illegal_state(
          "[PortableReaderBase::PortableReaderBase]", msg));
    }
    this->offset_ = input.position();
}

void
PortableReaderBase::set_position(const std::string& field_name,
                                 field_type const& field_type)
{
    if (raw_) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "PortableReader::getPosition ",
          "Cannot read Portable fields after getRawDataInput() is called!"));
    }
    if (!cd_->has_field(field_name)) {
        // TODO: if no field def found, java client reads nested position:
        // readNestedPosition(fieldName, type);
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "PortableReader::getPosition ",
          "Don't have a field named " + std::string(field_name)));
    }

    if (cd_->get_field_type(field_name) != field_type) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "PortableReader::getPosition ",
          "Field type did not matched for " + std::string(field_name)));
    }

    data_input_->position(offset_ + cd_->get_field(field_name).get_index() *
                                      util::Bits::INT_SIZE_IN_BYTES);
    int32_t pos = data_input_->read<int32_t>();

    data_input_->position(pos);
    int16_t len = data_input_->read<int16_t>();

    // name + len + type
    data_input_->position(pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1);
}

hazelcast::client::serialization::object_data_input&
PortableReaderBase::get_raw_data_input()
{
    if (!raw_) {
        data_input_->position(offset_ + cd_->get_field_count() *
                                          util::Bits::INT_SIZE_IN_BYTES);
        int32_t pos = data_input_->read<int32_t>();
        data_input_->position(pos);
    }
    raw_ = true;
    return *data_input_;
}

void
PortableReaderBase::end()
{
    data_input_->position(final_position_);
}

void
PortableReaderBase::check_factory_and_class(FieldDefinition fd,
                                            int32_t factory_id,
                                            int32_t class_id) const
{
    if (factory_id != fd.get_factory_id()) {
        char msg[100];
        util::hz_snprintf(msg,
                          100,
                          "Invalid factoryId! Expected: %d, Current: %d",
                          fd.get_factory_id(),
                          factory_id);
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "DefaultPortableReader::checkFactoryAndClass ", std::string(msg)));
    }
    if (class_id != fd.get_class_id()) {
        char msg[100];
        util::hz_snprintf(msg,
                          100,
                          "Invalid classId! Expected: %d, Current: %d",
                          fd.get_class_id(),
                          class_id);
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "DefaultPortableReader::checkFactoryAndClass ", std::string(msg)));
    }
}

MorphingPortableReader::MorphingPortableReader(
  PortableSerializer& portable_ser,
  object_data_input& input,
  std::shared_ptr<ClassDefinition> cd)
  : PortableReaderBase(portable_ser, input, cd)
{}

PortableSerializer::PortableSerializer(PortableContext& portable_context)
  : context_(portable_context)
{}

portable_reader
PortableSerializer::create_reader(object_data_input& input,
                                  int factory_id,
                                  int class_id,
                                  int version,
                                  int portable_version)
{

    int effectiveVersion = version;
    if (version < 0) {
        effectiveVersion = context_.get_version();
    }

    std::shared_ptr<ClassDefinition> cd =
      context_.lookup_class_definition(factory_id, class_id, effectiveVersion);
    if (cd == nullptr) {
        int begin = input.position();
        cd = context_.read_class_definition(
          input, factory_id, class_id, effectiveVersion);
        input.position(begin);
    }

    return portable_reader(
      *this, input, cd, effectiveVersion == portable_version);
}

int32_t
PortableSerializer::read_int(object_data_input& in) const
{
    return in.read<int32_t>();
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast

namespace std {
std::size_t
hash<hazelcast::client::hazelcast_json_value>::operator()(
  const hazelcast::client::hazelcast_json_value& object) const noexcept
{
    return std::hash<std::string>{}(object.to_string());
}

std::size_t
hash<hazelcast::client::serialization::pimpl::data>::operator()(
  const hazelcast::client::serialization::pimpl::data& val) const noexcept
{
    return std::hash<int>{}(val.hash());
}

std::size_t
hash<std::shared_ptr<hazelcast::client::serialization::pimpl::data>>::
operator()(const std::shared_ptr<hazelcast::client::serialization::pimpl::data>&
             val) const noexcept
{
    if (!val) {
        return std::hash<int>{}(-1);
    }
    return std::hash<int>{}(val->hash());
}

bool
equal_to<std::shared_ptr<hazelcast::client::serialization::pimpl::data>>::
operator()(
  std::shared_ptr<hazelcast::client::serialization::pimpl::data> const& lhs,
  std::shared_ptr<hazelcast::client::serialization::pimpl::data> const& rhs)
  const noexcept
{
    if (lhs == rhs) {
        return true;
    }

    if (!lhs || !rhs) {
        return false;
    }

    return lhs->to_byte_array() == rhs->to_byte_array();
}

bool
less<std::shared_ptr<hazelcast::client::serialization::pimpl::data>>::
operator()(
  const std::shared_ptr<hazelcast::client::serialization::pimpl::data>& lhs,
  const std::shared_ptr<hazelcast::client::serialization::pimpl::data>& rhs)
  const noexcept
{
    const hazelcast::client::serialization::pimpl::data* leftPtr = lhs.get();
    const hazelcast::client::serialization::pimpl::data* rightPtr = rhs.get();
    if (leftPtr == rightPtr) {
        return false;
    }

    if (leftPtr == NULL) {
        return true;
    }

    if (rightPtr == NULL) {
        return false;
    }

    return lhs->hash() < rhs->hash();
}
} // namespace std
