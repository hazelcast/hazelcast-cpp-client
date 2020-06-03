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
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/MurmurHash3.h"

namespace hazelcast {
    namespace client {
        HazelcastJsonValue::HazelcastJsonValue(std::string jsonString) : jsonString(std::move(jsonString)) {
        }

        HazelcastJsonValue::~HazelcastJsonValue() = default;

        const std::string &HazelcastJsonValue::toString() const {
            return jsonString;
        }

        bool HazelcastJsonValue::operator==(const HazelcastJsonValue &rhs) const {
            return jsonString == rhs.jsonString;
        }

        bool HazelcastJsonValue::operator!=(const HazelcastJsonValue &rhs) const {
            return !(rhs == *this);
        }

        std::ostream &operator<<(std::ostream &os, const HazelcastJsonValue &value) {
            os << "jsonString: " << value.jsonString;
            return os;
        }

        TypedData::TypedData() : ss(nullptr) {
        }

        TypedData::TypedData(serialization::pimpl::Data d,
                             serialization::pimpl::SerializationService &serializationService) : data(std::move(d)),
                                                                                                 ss(&serializationService) {}

        serialization::pimpl::ObjectType TypedData::getType() const {
            return ss->getObjectType(&data);
        }

        const serialization::pimpl::Data &TypedData::getData() const {
            return data;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            const auto& lhsData = lhs.getData();
            const auto& rhsData = rhs.getData();

            return lhsData < rhsData;
        }

        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
                    : defaultPortableWriter(defaultPortableWriter), classDefinitionWriter(nullptr), isDefaultWriter(true) {}

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
                    : defaultPortableWriter(nullptr), classDefinitionWriter(classDefinitionWriter),
                      isDefaultWriter(false) {}

            void PortableWriter::end() {
                if (isDefaultWriter)
                    return defaultPortableWriter->end();
                return classDefinitionWriter->end();
            }

            ObjectDataOutput &PortableWriter::getRawDataOutput() {
                if (isDefaultWriter)
                    return defaultPortableWriter->getRawDataOutput();
                return classDefinitionWriter->getRawDataOutput();
            }

            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), index(0), done(false) {}

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName,
                                                                             std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                                "Portable class id cannot be zero!"));
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldType::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
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
                FieldDefinition fieldDefinition(index++, fieldName, FieldType::TYPE_PORTABLE_ARRAY,
                                                def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addField(FieldDefinition &fieldDefinition) {
                check();
                int defIndex = fieldDefinition.getIndex();
                if (index != defIndex) {
                    char buf[100];
                    util::hz_snprintf(buf, 100, "Invalid field index. Index in definition:%d, being added at index:%d",
                                      defIndex, index);
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf));
                }
                index++;
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            std::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done = true;
                std::shared_ptr<ClassDefinition> cd(new ClassDefinition(factoryId, classId, version));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = fieldDefinitions.begin(); fdIt != fieldDefinitions.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ClassDefinitionBuilder::check",
                                                                                     "ClassDefinition is already built for " +
                                                                                     util::IOUtil::to_string(classId)));
                }
            }

            void ClassDefinitionBuilder::addField(const std::string &fieldName, FieldType const &fieldType) {
                check();
                FieldDefinition fieldDefinition(index++, fieldName, fieldType, version);
                fieldDefinitions.push_back(fieldDefinition);
            }

            int ClassDefinitionBuilder::getFactoryId() {
                return factoryId;
            }

            int ClassDefinitionBuilder::getClassId() {
                return classId;
            }

            int ClassDefinitionBuilder::getVersion() {
                return version;
            }

            FieldDefinition::FieldDefinition()
                    : index(0), classId(0), factoryId(0), version(-1) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int version)
                    : index(index), fieldName(fieldName), type(type), classId(0), factoryId(0), version(version) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int factoryId, int classId, int version)
                    : index(index), fieldName(fieldName), type(type), classId(classId), factoryId(factoryId),
                      version(version) {}

            const FieldType &FieldDefinition::getType() const {
                return type;
            }

            std::string FieldDefinition::getName() const {
                return fieldName;
            }

            int FieldDefinition::getIndex() const {
                return index;
            }

            int FieldDefinition::getFactoryId() const {
                return factoryId;
            }

            int FieldDefinition::getClassId() const {
                return classId;
            }

            void FieldDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(index);
                dataOutput.write<std::string>(fieldName);
                dataOutput.write<byte>(static_cast<int32_t>(type));
                dataOutput.write<int32_t>(factoryId);
                dataOutput.write<int32_t>(classId);
            }

            void FieldDefinition::readData(ObjectDataInput &dataInput) {
                index = dataInput.read<int32_t>();
                fieldName = dataInput.read<std::string>();
                type = static_cast<FieldType>(dataInput.read<byte>());
                factoryId = dataInput.read<int32_t>();
                classId = dataInput.read<int32_t>();
            }

            bool FieldDefinition::operator==(const FieldDefinition &rhs) const {
                return fieldName == rhs.fieldName &&
                       type == rhs.type &&
                       classId == rhs.classId &&
                       factoryId == rhs.factoryId &&
                       version == rhs.version;
            }

            bool FieldDefinition::operator!=(const FieldDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition) {
                os << "FieldDefinition{" << "index: " << definition.index << " fieldName: " << definition.fieldName
                   << " type: " << static_cast<int32_t>(definition.type) << " classId: " << definition.classId << " factoryId: "
                   << definition.factoryId << " version: " << definition.version;
                return os;
            }

            ObjectDataInput::ObjectDataInput(const std::vector<byte> &buffer, int offset,
                    pimpl::PortableSerializer &portableSer, pimpl::DataSerializer &dataSer,
                    std::shared_ptr<serialization::global_serializer> globalSerializer)
                    : pimpl::DataInput<std::vector<byte>>(buffer, offset), portableSerializer(portableSer), dataSerializer(dataSer),
                      globalSerializer_(std::move(globalSerializer)) {}

            ObjectDataOutput::ObjectDataOutput(bool dontWrite, pimpl::PortableSerializer *portableSer,
                                               std::shared_ptr<serialization::global_serializer> globalSerializer)
                    : DataOutput(dontWrite), portableSerializer(portableSer), globalSerializer_(std::move(globalSerializer)) {}

            PortableReader::PortableReader(pimpl::PortableSerializer &portableSer, ObjectDataInput &input,
                                           const std::shared_ptr<ClassDefinition> &cd, bool isDefaultReader)
                    : isDefaultReader(isDefaultReader) {
                if (isDefaultReader) {
                    defaultPortableReader = boost::make_optional(pimpl::DefaultPortableReader(portableSer, input, cd));
                } else {
                    morphingPortableReader = boost::make_optional(pimpl::MorphingPortableReader(portableSer, input, cd));
                }
            }

            ObjectDataInput &PortableReader::getRawDataInput() {
                if (isDefaultReader)
                    return defaultPortableReader->getRawDataInput();
                return morphingPortableReader->getRawDataInput();
            }

            void PortableReader::end() {
                if (isDefaultReader)
                    return defaultPortableReader->end();
                return morphingPortableReader->end();

            }

            ClassDefinition::ClassDefinition()
                    : factoryId(0), classId(0), version(-1), binary(new std::vector<byte>) {
            }

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), binary(new std::vector<byte>) {
            }

            void ClassDefinition::addFieldDef(FieldDefinition &fd) {
                fieldDefinitionsMap[fd.getName()] = fd;
            }

            const FieldDefinition &ClassDefinition::getField(const std::string &name) const {
                auto it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return it->second;
                }
                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ClassDefinition::getField",
                        (boost::format("Invalid field name: '%1%' for ClassDefinition {id: %2%, version: %3%}")
                        %name %classId %version).str()));
            }

            bool ClassDefinition::hasField(const std::string &fieldName) const {
                return fieldDefinitionsMap.find(fieldName) != fieldDefinitionsMap.end();
            }

            FieldType ClassDefinition::getFieldType(const std::string &fieldName) const {
                FieldDefinition const &fd = getField(fieldName);
                return fd.getType();
            }

            int ClassDefinition::getFieldCount() const {
                return (int) fieldDefinitionsMap.size();
            }


            int ClassDefinition::getFactoryId() const {
                return factoryId;
            }

            int ClassDefinition::getClassId() const {
                return classId;
            }

            int ClassDefinition::getVersion() const {
                return version;
            }

            void ClassDefinition::setVersionIfNotSet(int newVersion) {
                if (getVersion() < 0) {
                    this->version = newVersion;
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(factoryId);
                dataOutput.write<int32_t>(classId);
                dataOutput.write<int32_t>(version);
                dataOutput.write<int16_t>(fieldDefinitionsMap.size());
                for (auto &entry : fieldDefinitionsMap) {
                    entry.second.writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(ObjectDataInput &dataInput) {
                factoryId = dataInput.read<int32_t>();
                classId = dataInput.read<int32_t>();
                version = dataInput.read<int32_t>();
                int size = dataInput.read<int16_t>();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
            }

            bool ClassDefinition::operator==(const ClassDefinition &rhs) const {
                return factoryId == rhs.factoryId &&
                       classId == rhs.classId &&
                       version == rhs.version &&
                       fieldDefinitionsMap == rhs.fieldDefinitionsMap;
            }

            bool ClassDefinition::operator!=(const ClassDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition) {
                os << "ClassDefinition{" << "factoryId: " << definition.factoryId << " classId: " << definition.classId
                   << " version: "
                   << definition.version << " fieldDefinitions: {";

                for (auto &entry : definition.fieldDefinitionsMap) {
                    os << entry.second;
                }
                os << "} }";
                return os;
            }

            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder(builder), context(portableContext), emptyDataOutput(true) {}

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
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
                        serializationConfig(serializationConf) {}

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
                    return serializationConfig.getPortableVersion();
                }

                ClassDefinitionContext &PortableContext::getClassDefinitionContext(int factoryId) {
                    std::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = std::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(factoryId, this));
                        std::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId,
                                                                                                         value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

                const SerializationConfig &PortableContext::getSerializationConfig() const {
                    return serializationConfig;
                }

                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig), serializationConfig(serializationConfig),
                          portableSerializer(portableContext) {}

                DefaultPortableWriter::DefaultPortableWriter(PortableSerializer &portableSer,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw(false), portableSerializer(portableSer), objectDataOutput(output), 
                        begin(objectDataOutput.position()), cd(cd) {
                    // room for final offset
                    objectDataOutput.write<int32_t>(0);

                    objectDataOutput.write<int32_t>(cd->getFieldCount());

                    offset = objectDataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    objectDataOutput.writeZeroBytes(fieldIndexesLength);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const std::string &fieldName, FieldType fieldType) {
                    if (raw) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                                         "Cannot write Portable fields after getRawDataOutput() is called!"));
                    }

                    try {
                        FieldDefinition const &fd = cd->getField(fieldName);

                        if (writtenFields.find(fieldName) != writtenFields.end()) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                               "Field '" + std::string(fieldName) +
                                                                               "' has already been written!"));
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = objectDataOutput.position();
                        int32_t index = fd.getIndex();
                        objectDataOutput.writeAt(offset + index * util::Bits::INT_SIZE_IN_BYTES, static_cast<int32_t>(pos));
                        objectDataOutput.write(static_cast<int16_t>(fieldName.size()));
                        objectDataOutput.writeBytes(fieldName);
                        objectDataOutput.write<byte>(static_cast<byte>(fieldType));

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}. Error:";
                        error << iae.what();

                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("PortableWriter::setPosition", error.str()));
                    }

                }

                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = objectDataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        objectDataOutput.position(offset + index * util::Bits::INT_SIZE_IN_BYTES);
                        objectDataOutput.write(static_cast<int32_t>(pos));
                        objectDataOutput.position(pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    objectDataOutput.writeAt(begin, static_cast<int32_t>(objectDataOutput.position()));
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
                    return portableSerializer;
                }

                DataSerializer &SerializationService::getDataSerializer() {
                    return dataSerializer;
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

                Data::Data() : cachedHashValue(-1) {}
                
                Data::Data(std::vector<byte> buffer) : data(std::move(buffer)), cachedHashValue(-1) {
                    size_t size = data.size();
                    if (size > 0 && size < Data::DATA_OVERHEAD) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Data::setBuffer")
                                << "Provided buffer should be either empty or should contain more than "
                                << Data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size).build();
                    }

                    cachedHashValue = calculateHash();
                }

                size_t Data::dataSize() const {
                    return (size_t) std::max<int>((int) totalSize() - (int) Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data.size();
                }

                int Data::getPartitionHash() const {
                    return cachedHashValue;
                }

                bool Data::hasPartitionHash() const {
                    return data.size() >= Data::DATA_OVERHEAD &&
                           *reinterpret_cast<const int32_t *>(&data[PARTITION_HASH_OFFSET]) != 0;
                }

                const std::vector<byte> &Data::toByteArray() const {
                    return data;
                }

                int32_t Data::getType() const {
                    if (totalSize() == 0) {
                        return static_cast<int32_t>(SerializationConstants::CONSTANT_TYPE_NULL);
                    }
                    return util::Bits::readIntB(data, Data::TYPE_OFFSET);
                }

                int Data::hash() const {
                    return cachedHashValue;
                }

                int Data::calculateHash() const {
                    size_t size = dataSize();
                    if (size == 0) {
                        return 0;
                    }

                    if (hasPartitionHash()) {
                        return util::Bits::readIntB(data, Data::PARTITION_HASH_OFFSET);
                    }

                    return util::MurmurHash3_x86_32((void *) &((data)[Data::DATA_OFFSET]), (int) size);
                }

                bool Data::operator<(const Data &rhs) const {
                    return cachedHashValue < rhs.cachedHashValue;
                }

                bool operator==(const Data &lhs, const Data &rhs) {
                    return lhs.data == rhs.data;
                }

                template<typename T>
                int DataInput<T>::position() {
                    return pos;
                }

                template<typename T>
                void DataInput<T>::position(int position) {
                    if (position > pos) {
                        checkAvailable((size_t) (position - pos));
                    }
                    pos = position;
                }

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factoryId(factoryId), portableContext(portableContext) {}

                int ClassDefinitionContext::getClassVersion(int classId) {
                    std::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    std::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, std::shared_ptr<int>(
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
                    return versionedDefinitions.get(key);

                }

                std::shared_ptr<ClassDefinition>
                ClassDefinitionContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return std::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factoryId) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factoryId << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    std::shared_ptr<ClassDefinition> currentCd = versionedDefinitions.putIfAbsent(versionedClassId, cd);
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
                        : cd(cd), dataInput(&input), portableSerializer(&portableSer), raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.read<int32_t>();
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
                    this->offset = input.position();
                }
                
                void PortableReaderBase::setPosition(const std::string &fieldName, FieldType const &fieldType) {
                    if (raw) {
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

                    dataInput->position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput->read<int32_t>();

                    dataInput->position(pos);
                    int16_t len = dataInput->read<int16_t>();

                    // name + len + type
                    dataInput->position(pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1);
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput->position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput->read<int32_t>();
                        dataInput->position(pos);
                    }
                    raw = true;
                    return *dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput->position(finalPosition);
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

                PortableSerializer::PortableSerializer(PortableContext &portableContext) : context(portableContext) {}

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == nullptr) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
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

bool std::less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()(
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


