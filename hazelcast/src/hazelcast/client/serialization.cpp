/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        HazelcastJsonValue::HazelcastJsonValue(std::string jsonString) : json_string_(std::move(jsonString)) {
        }

        HazelcastJsonValue::~HazelcastJsonValue() = default;

        const std::string &HazelcastJsonValue::toString() const {
            return json_string_;
        }

        bool HazelcastJsonValue::operator==(const HazelcastJsonValue &rhs) const {
            return json_string_ == rhs.json_string_;
        }

        bool HazelcastJsonValue::operator!=(const HazelcastJsonValue &rhs) const {
            return !(rhs == *this);
        }

        bool HazelcastJsonValue::operator<(const HazelcastJsonValue &rhs) const {
            return json_string_ < rhs.json_string_;
        }

        std::ostream &operator<<(std::ostream &os, const HazelcastJsonValue &value) {
            os << "jsonString: " << value.json_string_;
            return os;
        }

        TypedData::TypedData() : ss_(nullptr) {
        }

        TypedData::TypedData(serialization::pimpl::Data d,
                             serialization::pimpl::SerializationService &serializationService) : data_(std::move(d)),
                                                                                                 ss_(&serializationService) {}

        serialization::pimpl::ObjectType TypedData::getType() const {
            return ss_->getObjectType(&data_);
        }

        const serialization::pimpl::Data &TypedData::getData() const {
            return data_;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            const auto& lhsData = lhs.getData();
            const auto& rhsData = rhs.getData();

            return lhsData < rhsData;
        }

        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
                    : default_portable_writer_(defaultPortableWriter), class_definition_writer_(nullptr), is_default_writer_(true) {}

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
                    : default_portable_writer_(nullptr), class_definition_writer_(classDefinitionWriter),
                      is_default_writer_(false) {}

            void PortableWriter::end() {
                if (is_default_writer_)
                    return default_portable_writer_->end();
                return class_definition_writer_->end();
            }

            ObjectDataOutput &PortableWriter::getRawDataOutput() {
                if (is_default_writer_)
                    return default_portable_writer_->getRawDataOutput();
                return class_definition_writer_->getRawDataOutput();
            }

            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
                    : factory_id_(factoryId), class_id_(classId), version_(version), index_(0), done_(false) {}

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName,
                                                                             std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                                "Portable class id cannot be zero!"));
                }
                FieldDefinition fieldDefinition(index_++, fieldName, FieldType::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                field_definitions_.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableArrayField(const std::string &fieldName,
                                                                                  std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                                "Portable class id cannot be zero!"));
                }
                FieldDefinition fieldDefinition(index_++, fieldName, FieldType::TYPE_PORTABLE_ARRAY,
                                                def->getFactoryId(), def->getClassId(), def->getVersion());
                field_definitions_.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addField(FieldDefinition &fieldDefinition) {
                check();
                int defIndex = fieldDefinition.getIndex();
                if (index_ != defIndex) {
                    char buf[100];
                    util::hz_snprintf(buf, 100, "Invalid field index. Index in definition:%d, being added at index:%d",
                                      defIndex, index_);
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf));
                }
                index_++;
                field_definitions_.push_back(fieldDefinition);
                return *this;
            }

            std::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done_ = true;
                std::shared_ptr<ClassDefinition> cd(new ClassDefinition(factory_id_, class_id_, version_));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = field_definitions_.begin(); fdIt != field_definitions_.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done_) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ClassDefinitionBuilder::check",
                                                                                     "ClassDefinition is already built for " +
                                                                                     util::IOUtil::to_string(class_id_)));
                }
            }

            void ClassDefinitionBuilder::addField(const std::string &fieldName, FieldType const &fieldType) {
                check();
                FieldDefinition fieldDefinition(index_++, fieldName, fieldType, version_);
                field_definitions_.push_back(fieldDefinition);
            }

            int ClassDefinitionBuilder::getFactoryId() {
                return factory_id_;
            }

            int ClassDefinitionBuilder::getClassId() {
                return class_id_;
            }

            int ClassDefinitionBuilder::getVersion() {
                return version_;
            }

            FieldDefinition::FieldDefinition()
                    : index_(0), class_id_(0), factory_id_(0), version_(-1) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int version)
                    : index_(index), field_name_(fieldName), type_(type), class_id_(0), factory_id_(0), version_(version) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int factoryId, int classId, int version)
                    : index_(index), field_name_(fieldName), type_(type), class_id_(classId), factory_id_(factoryId),
                      version_(version) {}

            const FieldType &FieldDefinition::getType() const {
                return type_;
            }

            std::string FieldDefinition::getName() const {
                return field_name_;
            }

            int FieldDefinition::getIndex() const {
                return index_;
            }

            int FieldDefinition::getFactoryId() const {
                return factory_id_;
            }

            int FieldDefinition::getClassId() const {
                return class_id_;
            }

            void FieldDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(index_);
                dataOutput.write<std::string>(field_name_);
                dataOutput.write<byte>(static_cast<int32_t>(type_));
                dataOutput.write<int32_t>(factory_id_);
                dataOutput.write<int32_t>(class_id_);
            }

            void FieldDefinition::readData(ObjectDataInput &dataInput) {
                index_ = dataInput.read<int32_t>();
                field_name_ = dataInput.read<std::string>();
                type_ = static_cast<FieldType>(dataInput.read<byte>());
                factory_id_ = dataInput.read<int32_t>();
                class_id_ = dataInput.read<int32_t>();
            }

            bool FieldDefinition::operator==(const FieldDefinition &rhs) const {
                return field_name_ == rhs.field_name_ &&
                       type_ == rhs.type_ &&
                       class_id_ == rhs.class_id_ &&
                       factory_id_ == rhs.factory_id_ &&
                       version_ == rhs.version_;
            }

            bool FieldDefinition::operator!=(const FieldDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition) {
                os << "FieldDefinition{" << "index: " << definition.index_ << " fieldName: " << definition.field_name_
                   << " type: " << static_cast<int32_t>(definition.type_) << " classId: " << definition.class_id_ << " factoryId: "
                   << definition.factory_id_ << " version: " << definition.version_;
                return os;
            }

            ObjectDataInput::ObjectDataInput(const std::vector<byte> &buffer, int offset,
                    pimpl::PortableSerializer &portableSer, pimpl::DataSerializer &dataSer,
                    std::shared_ptr<serialization::global_serializer> globalSerializer)
                    : pimpl::DataInput<std::vector<byte>>(buffer, offset), portable_serializer_(portableSer), data_serializer_(dataSer),
                      global_serializer_(std::move(globalSerializer)) {}

            ObjectDataOutput::ObjectDataOutput(bool dontWrite, pimpl::PortableSerializer *portableSer,
                                               std::shared_ptr<serialization::global_serializer> globalSerializer)
                    : DataOutput(dontWrite), portable_serializer_(portableSer), global_serializer_(std::move(globalSerializer)) {}

            PortableReader::PortableReader(pimpl::PortableSerializer &portableSer, ObjectDataInput &input,
                                           const std::shared_ptr<ClassDefinition> &cd, bool isDefaultReader)
                    : is_default_reader_(isDefaultReader) {
                if (isDefaultReader) {
                    default_portable_reader_ = boost::make_optional(pimpl::DefaultPortableReader(portableSer, input, cd));
                } else {
                    morphing_portable_reader_ = boost::make_optional(pimpl::MorphingPortableReader(portableSer, input, cd));
                }
            }

            ObjectDataInput &PortableReader::getRawDataInput() {
                if (is_default_reader_)
                    return default_portable_reader_->getRawDataInput();
                return morphing_portable_reader_->getRawDataInput();
            }

            template<>
            void ObjectDataOutput::writeObject(const char *object) {
                if (!object) {
                    write<int32_t>(static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_NULL));
                    return;
                }
                writeObject<std::string>(std::string(object));
            }

            void PortableReader::end() {
                if (is_default_reader_)
                    return default_portable_reader_->end();
                return morphing_portable_reader_->end();

            }

            ClassDefinition::ClassDefinition()
                    : factory_id_(0), class_id_(0), version_(-1), binary_(new std::vector<byte>) {
            }

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
                    : factory_id_(factoryId), class_id_(classId), version_(version), binary_(new std::vector<byte>) {
            }

            void ClassDefinition::addFieldDef(FieldDefinition &fd) {
                field_definitions_map_[fd.getName()] = fd;
            }

            const FieldDefinition &ClassDefinition::getField(const std::string &name) const {
                auto it = field_definitions_map_.find(name);
                if (it != field_definitions_map_.end()) {
                    return it->second;
                }
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ClassDefinition::getField",
                        (boost::format("Invalid field name: '%1%' for ClassDefinition {id: %2%, version: %3%}")
                        %name %class_id_ %version_).str()));
            }

            bool ClassDefinition::hasField(const std::string &fieldName) const {
                return field_definitions_map_.find(fieldName) != field_definitions_map_.end();
            }

            FieldType ClassDefinition::getFieldType(const std::string &fieldName) const {
                FieldDefinition const &fd = getField(fieldName);
                return fd.getType();
            }

            int ClassDefinition::getFieldCount() const {
                return (int) field_definitions_map_.size();
            }


            int ClassDefinition::getFactoryId() const {
                return factory_id_;
            }

            int ClassDefinition::getClassId() const {
                return class_id_;
            }

            int ClassDefinition::getVersion() const {
                return version_;
            }

            void ClassDefinition::setVersionIfNotSet(int newVersion) {
                if (getVersion() < 0) {
                    this->version_ = newVersion;
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(factory_id_);
                dataOutput.write<int32_t>(class_id_);
                dataOutput.write<int32_t>(version_);
                dataOutput.write<int16_t>(field_definitions_map_.size());
                for (auto &entry : field_definitions_map_) {
                    entry.second.writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(ObjectDataInput &dataInput) {
                factory_id_ = dataInput.read<int32_t>();
                class_id_ = dataInput.read<int32_t>();
                version_ = dataInput.read<int32_t>();
                int size = dataInput.read<int16_t>();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
            }

            bool ClassDefinition::operator==(const ClassDefinition &rhs) const {
                return factory_id_ == rhs.factory_id_ &&
                       class_id_ == rhs.class_id_ &&
                       version_ == rhs.version_ &&
                       field_definitions_map_ == rhs.field_definitions_map_;
            }

            bool ClassDefinition::operator!=(const ClassDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition) {
                os << "ClassDefinition{" << "factoryId: " << definition.factory_id_ << " classId: " << definition.class_id_
                   << " version: "
                   << definition.version_ << " fieldDefinitions: {";

                for (auto &entry : definition.field_definitions_map_) {
                    os << entry.second;
                }
                os << "} }";
                return os;
            }

            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder_(builder), context_(portableContext), empty_data_output_(true) {}

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder_.build();
                    return context_.registerClassDefinition(cd);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return empty_data_output_;
                }

                void ClassDefinitionWriter::end() {}

                DataOutput::DataOutput(bool dontWrite) :isNoWrite(dontWrite) {
                    if (isNoWrite) {
                        outputStream.reserve(0);
                    } else {
                        outputStream.reserve(DEFAULT_SIZE);
                    }
                }

                template<>
                void DataOutput::write(byte i) {
                    if (isNoWrite) { return; }
                    outputStream.push_back(i);
                }

                template<>
                void DataOutput::write(char i) {
                    if (isNoWrite) { return; }
                    // C++ `char` is one byte only, `char16_t` is two bytes
                    write<byte>(0);
                    write<byte>(i);
                }

                template<>
                void DataOutput::write(char16_t i) {
                    if (isNoWrite) { return; }
                    write<byte>(static_cast<byte>(i >> 8));
                    write<byte>(i);
                }

                template<>
                void DataOutput::write(int16_t value) {
                    if (isNoWrite) { return; }
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(int32_t v) {
                    if (isNoWrite) { return; }
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(int64_t l) {
                    if (isNoWrite) { return; }
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(float x) {
                    if (isNoWrite) { return; }
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    write<int32_t>(u.i);
                }

                template<>
                void DataOutput::write(double v) {
                    if (isNoWrite) { return; }
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    write<int64_t>(u.l);
                }

                template<>
                void DataOutput::write(boost::uuids::uuid v) {
                    if (isNoWrite) { return; }
                    outputStream.insert(outputStream.end(), v.data, v.data + util::Bits::UUID_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(bool value) {
                    if (isNoWrite) { return; }
                    write<byte>(value);
                }

                template<>
                void DataOutput::write(const std::string &str) {
                    if (isNoWrite) { return; }
                    int32_t len = util::UTFUtil::isValidUTF8(str);
                    if (len < 0) {
                        BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::UTFDataFormatException>(
                                "DataOutput::write")
                                << "String \"" << str << "\" is not UTF-8 formatted !!!").build());
                    }

                    write<int32_t>(len);
                    if (len > 0) {
                        outputStream.insert(outputStream.end(), str.begin(), str.end());
                    }
                }

                template<>
                void DataOutput::write(const HazelcastJsonValue &value) {
                    if (isNoWrite) { return; }
                    write<std::string>(value.toString());
                }

                ObjectType::ObjectType() : typeId(SerializationConstants::CONSTANT_TYPE_NULL), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << static_cast<int32_t>(type.typeId) << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.read<int32_t>();
                }

                PortableContext::PortableContext(const SerializationConfig &serializationConf) :
                        serialization_config_(serializationConf) {}

                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::readClassDefinition(ObjectDataInput &in, int factoryId, int classId, int version) {
                    bool shouldRegister = true;
                    ClassDefinitionBuilder builder(factoryId, classId, version);

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
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(static_cast<FieldType>(in.read<byte>()));
                        std::string name((char *) &(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        int fieldVersion = version;
                        if (type == FieldType::TYPE_PORTABLE) {
                            // is null
                            if (in.read<bool>()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.read<int32_t>();
                            fieldClassId = in.read<int32_t>();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                fieldVersion = in.read<int32_t>();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldType::TYPE_PORTABLE_ARRAY) {
                            int k = in.read<int32_t>();
                            if (k > 0) {
                                fieldFactoryId = in.read<int32_t>();
                                fieldClassId = in.read<int32_t>();

                                int p = in.read<int32_t>();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                fieldVersion = in.read<int32_t>();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId, fieldVersion);
                        builder.addField(fieldDef);
                    }
                    std::shared_ptr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                int PortableContext::getVersion() {
                    return serialization_config_.getPortableVersion();
                }

                ClassDefinitionContext &PortableContext::getClassDefinitionContext(int factoryId) {
                    std::shared_ptr<ClassDefinitionContext> value = class_def_context_map_.get(factoryId);
                    if (value == NULL) {
                        value = std::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(factoryId, this));
                        std::shared_ptr<ClassDefinitionContext> current = class_def_context_map_.putIfAbsent(factoryId,
                                                                                                         value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

                const SerializationConfig &PortableContext::getSerializationConfig() const {
                    return serialization_config_;
                }

                SerializationService::SerializationService(const SerializationConfig &config)
                        : serialization_config_(config), portable_context_(serialization_config_),
                          portable_serializer_(portable_context_) {}

                DefaultPortableWriter::DefaultPortableWriter(PortableSerializer &portableSer,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw_(false), portable_serializer_(portableSer), object_data_output_(output), 
                        begin_(object_data_output_.position()), cd_(cd) {
                    // room for final offset
                    object_data_output_.write<int32_t>(0);

                    object_data_output_.write<int32_t>(cd->getFieldCount());

                    offset_ = object_data_output_.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    object_data_output_.writeZeroBytes(fieldIndexesLength);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const std::string &fieldName, FieldType fieldType) {
                    if (raw_) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                                         "Cannot write Portable fields after getRawDataOutput() is called!"));
                    }

                    try {
                        FieldDefinition const &fd = cd_->getField(fieldName);

                        if (written_fields_.find(fieldName) != written_fields_.end()) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                               "Field '" + std::string(fieldName) +
                                                                               "' has already been written!"));
                        }

                        written_fields_.insert(fieldName);
                        size_t pos = object_data_output_.position();
                        int32_t index = fd.getIndex();
                        object_data_output_.writeAt(offset_ + index * util::Bits::INT_SIZE_IN_BYTES, static_cast<int32_t>(pos));
                        object_data_output_.write(static_cast<int16_t>(fieldName.size()));
                        object_data_output_.writeBytes(fieldName);
                        object_data_output_.write<byte>(static_cast<byte>(fieldType));

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd_->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd_->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd_->getVersion()) << "}. Error:";
                        error << iae.what();

                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("PortableWriter::setPosition", error.str()));
                    }

                }

                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw_) {
                        size_t pos = object_data_output_.position();
                        int32_t index = cd_->getFieldCount(); // last index
                        object_data_output_.writeAt(offset_ + index * util::Bits::INT_SIZE_IN_BYTES, static_cast<int32_t>(pos));
                    }
                    raw_ = true;
                    return object_data_output_;
                }

                void DefaultPortableWriter::end() {
                    object_data_output_.writeAt(begin_, static_cast<int32_t>(object_data_output_.position()));
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 &&
                           data.getType() == static_cast<int32_t>(SerializationConstants::CONSTANT_TYPE_NULL);
                }

                template<>
                Data SerializationService::toData(const char *object) {
                    if (!object) {
                        return toData<std::string>(nullptr);
                    }
                    std::string str(object);
                    return toData<std::string>(str);
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        type.typeId = SerializationConstants::CONSTANT_TYPE_NULL;
                        return type;
                    }

                    type.typeId = static_cast<SerializationConstants>(data->getType());

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                        SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        // 8 (Data Header) = Hash(4-bytes) + Data TypeId(4 bytes)
                        DataInput<std::vector<byte>> dataInput(data->toByteArray(), 8);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = dataInput.read<bool>();
                            if (!identified) {
                                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                              "SerializationService::getObjectType",
                                                              " DataSerializable is not 'identified data'"));
                            }
                        }

                        type.factoryId = dataInput.read<int32_t>();
                        type.classId = dataInput.read<int32_t>();
                    }

                    return type;
                }

                void SerializationService::dispose() {
                }

                PortableSerializer &SerializationService::getPortableSerializer() {
                    return portable_serializer_;
                }

                DataSerializer &SerializationService::getDataSerializer() {
                    return data_serializer_;
                }

                ObjectDataOutput SerializationService::newOutputStream() {
                    return ObjectDataOutput(false, &portable_serializer_, serialization_config_.getGlobalSerializer());
                }

                template<>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    auto data = object->getData();
                    return Data(data);
                }

                //first 4 byte is partition hash code and next last 4 byte is type id
                unsigned int Data::PARTITION_HASH_OFFSET = 0;

                unsigned int Data::TYPE_OFFSET = Data::PARTITION_HASH_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OFFSET = Data::TYPE_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OVERHEAD = Data::DATA_OFFSET;

                Data::Data() : cached_hash_value_(-1) {}
                
                Data::Data(std::vector<byte> buffer) : data_(std::move(buffer)), cached_hash_value_(-1) {
                    size_t size = data_.size();
                    if (size > 0 && size < Data::DATA_OVERHEAD) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Data::setBuffer")
                                << "Provided buffer should be either empty or should contain more than "
                                << Data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size).build();
                    }

                    cached_hash_value_ = calculateHash();
                }

                size_t Data::dataSize() const {
                    return (size_t) std::max<int>((int) totalSize() - (int) Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data_.size();
                }

                int Data::getPartitionHash() const {
                    return cached_hash_value_;
                }

                bool Data::hasPartitionHash() const {
                    return data_.size() >= Data::DATA_OVERHEAD &&
                           *reinterpret_cast<const int32_t *>(&data_[PARTITION_HASH_OFFSET]) != 0;
                }

                const std::vector<byte> &Data::toByteArray() const {
                    return data_;
                }

                int32_t Data::getType() const {
                    if (totalSize() == 0) {
                        return static_cast<int32_t>(SerializationConstants::CONSTANT_TYPE_NULL);
                    }
                    return util::Bits::readIntB(data_, Data::TYPE_OFFSET);
                }

                int Data::hash() const {
                    return cached_hash_value_;
                }

                int Data::calculateHash() const {
                    size_t size = dataSize();
                    if (size == 0) {
                        return 0;
                    }

                    if (hasPartitionHash()) {
                        return util::Bits::readIntB(data_, Data::PARTITION_HASH_OFFSET);
                    }

                    return util::MurmurHash3_x86_32((void *) &((data_)[Data::DATA_OFFSET]), (int) size);
                }

                bool Data::operator<(const Data &rhs) const {
                    return cached_hash_value_ < rhs.cached_hash_value_;
                }

                bool operator==(const Data &lhs, const Data &rhs) {
                    return lhs.data_ == rhs.data_;
                }

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factory_id_(factoryId), portable_context_(portableContext) {}

                int ClassDefinitionContext::getClassVersion(int classId) {
                    std::shared_ptr<int> version = current_class_versions_.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    std::shared_ptr<int> current = current_class_versions_.putIfAbsent(classId, std::shared_ptr<int>(
                            new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        BOOST_THROW_EXCEPTION(
                                exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion",
                                                                    error.str()));
                    }
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versioned_definitions_.get(key);

                }

                std::shared_ptr<ClassDefinition>
                ClassDefinitionContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return std::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factory_id_) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factory_id_ << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portable_context_->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    std::shared_ptr<ClassDefinition> currentCd = versioned_definitions_.putIfAbsent(versionedClassId, cd);
                    if (currentCd.get() == NULL) {
                        return cd;
                    }

                    if (currentCd.get() != cd.get() && *currentCd != *cd) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition")
                                << "Incompatible class-definitions with same class-id: " << *cd << " VS "
                                << *currentCd).build();
                    }

                    return currentCd;
                }

                int64_t ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((int64_t) x) << 32 | (((int64_t) y) & 0xFFFFFFFL);
                }

                DefaultPortableReader::DefaultPortableReader(PortableSerializer &portableSer,
                                                             ObjectDataInput &input,
                                                             std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableSer, input, cd) {}

                PortableReaderBase::PortableReaderBase(PortableSerializer &portableSer, ObjectDataInput &input,
                                                       std::shared_ptr<ClassDefinition> cd)
                        : cd(cd), dataInput(&input), portableSerializer(&portableSer), raw_(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        final_position_ = input.read<int32_t>();
                        // field count
                        fieldCount = input.read<int32_t>();
                    } catch (exception::IException &e) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "[PortableReaderBase::PortableReaderBase]", e.what()));
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::hz_snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount,
                                          cd->getFieldCount());
                        BOOST_THROW_EXCEPTION(
                                exception::IllegalStateException("[PortableReaderBase::PortableReaderBase]",
                                                                 msg));
                    }
                    this->offset_ = input.position();
                }
                
                void PortableReaderBase::setPosition(const std::string &fieldName, FieldType const &fieldType) {
                    if (raw_) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Cannot read Portable fields after getRawDataInput() is called!"));
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Don't have a field named " +
                                                                                         std::string(fieldName)));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Field type did not matched for " +
                                                                                         std::string(fieldName)));
                    }

                    dataInput->position(offset_ + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput->read<int32_t>();

                    dataInput->position(pos);
                    int16_t len = dataInput->read<int16_t>();

                    // name + len + type
                    dataInput->position(pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1);
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw_) {
                        dataInput->position(offset_ + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput->read<int32_t>();
                        dataInput->position(pos);
                    }
                    raw_ = true;
                    return *dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput->position(final_position_);
                }

                void
                PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int32_t factoryId, int32_t classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(),
                                          factoryId);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableReader::checkFactoryAndClass ",
                                                              std::string(msg)));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(),
                                          classId);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableReader::checkFactoryAndClass ",
                                                              std::string(msg)));
                    }
                }

                MorphingPortableReader::MorphingPortableReader(PortableSerializer &portableSer, ObjectDataInput &input,
                                                               std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableSer, input, cd) {}

                PortableSerializer::PortableSerializer(PortableContext &portableContext) : context_(portableContext) {}

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context_.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context_.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == nullptr) {
                        int begin = input.position();
                        cd = context_.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    return PortableReader(*this, input, cd, effectiveVersion == portableVersion);
                }

                int32_t PortableSerializer::readInt(ObjectDataInput &in) const {
                    return in.read<int32_t>();
                }
            }
        }
    }
}

namespace std {
    std::size_t hash<hazelcast::client::HazelcastJsonValue>::operator()(
            const hazelcast::client::HazelcastJsonValue &object) const noexcept {
        return std::hash<std::string>{}(object.toString());
    }

    std::size_t hash<hazelcast::client::serialization::pimpl::Data>::operator()
            (const hazelcast::client::serialization::pimpl::Data &val) const noexcept {
        return std::hash<int>{}(val.hash());
    }

    std::size_t hash<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()
        (const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &val) const noexcept {
        if (!val) {
            return std::hash<int>{}(-1);
        }
        return std::hash<int>{}(val->hash());
    }

    bool equal_to<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()
            (std::shared_ptr<hazelcast::client::serialization::pimpl::Data> const &lhs,
            std::shared_ptr<hazelcast::client::serialization::pimpl::Data> const &rhs) const noexcept {
        if (lhs == rhs) {
            return true;
        }

        if (!lhs || !rhs) {
            return false;
        }

        return lhs->toByteArray() == rhs->toByteArray();
    }

    bool less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()(
            const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
            const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept {
        const hazelcast::client::serialization::pimpl::Data *leftPtr = lhs.get();
        const hazelcast::client::serialization::pimpl::Data *rightPtr = rhs.get();
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
}


