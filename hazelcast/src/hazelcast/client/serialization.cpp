/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/VersionedPortable.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/MurmurHash3.h"

namespace hazelcast {
    namespace client {
        HazelcastJsonValue::HazelcastJsonValue(const std::string &jsonString) : jsonString(jsonString) {
        }

        HazelcastJsonValue::~HazelcastJsonValue() {
        }

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

        TypedData::TypedData() : ss(NULL) {
        }

        TypedData::TypedData(std::unique_ptr<serialization::pimpl::Data> &data,
                             serialization::pimpl::SerializationService &serializationService) : data(std::move(data)),
                                                                                                 ss(&serializationService) {
        }

        TypedData::TypedData(const std::shared_ptr<serialization::pimpl::Data> &data,
                             serialization::pimpl::SerializationService &serializationService) : data(data),
                                                                                                 ss(&serializationService) {

        }

        TypedData::~TypedData() {}

        const serialization::pimpl::ObjectType TypedData::getType() const {
            return ss->getObjectType(data.get());
        }

        const std::shared_ptr<serialization::pimpl::Data> TypedData::getData() const {
            return data;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            const serialization::pimpl::Data *lhsData = lhs.getData().get();
            const serialization::pimpl::Data *rhsData = rhs.getData().get();
            if (lhsData == NULL) {
                return true;
            }

            if (rhsData == NULL) {
                return false;
            }

            return *lhsData < *rhsData;
        }

        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
                    : defaultPortableWriter(defaultPortableWriter), classDefinitionWriter(NULL), isDefaultWriter(true) {

            }

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
                    : defaultPortableWriter(NULL), classDefinitionWriter(classDefinitionWriter),
                      isDefaultWriter(false) {

            }

            void PortableWriter::writeInt(const char *fieldName, int32_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeInt(fieldName, value);
                return classDefinitionWriter->writeInt(fieldName, value);
            }

            void PortableWriter::writeLong(const char *fieldName, int64_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLong(fieldName, value);
                return classDefinitionWriter->writeLong(fieldName, value);
            }

            void PortableWriter::writeBoolean(const char *fieldName, bool value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeBoolean(fieldName, value);
                return classDefinitionWriter->writeBoolean(fieldName, value);
            }

            void PortableWriter::writeByte(const char *fieldName, byte value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByte(fieldName, value);
                return classDefinitionWriter->writeByte(fieldName, value);
            }

            void PortableWriter::writeChar(const char *fieldName, int32_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeChar(fieldName, value);
                return classDefinitionWriter->writeChar(fieldName, value);
            }

            void PortableWriter::writeDouble(const char *fieldName, double value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDouble(fieldName, value);
                return classDefinitionWriter->writeDouble(fieldName, value);
            }

            void PortableWriter::writeFloat(const char *fieldName, float value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloat(fieldName, value);
                return classDefinitionWriter->writeFloat(fieldName, value);
            }

            void PortableWriter::writeShort(const char *fieldName, int16_t value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShort(fieldName, value);
                return classDefinitionWriter->writeShort(fieldName, value);
            }

            void PortableWriter::writeUTF(const char *fieldName, const std::string *str) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeUTF(fieldName, str);
                return classDefinitionWriter->writeUTF(fieldName, str);
            }

            void PortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByteArray(fieldName, data);
                return classDefinitionWriter->writeByteArray(fieldName, data);
            }

            void PortableWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeBooleanArray(fieldName, data);
                return classDefinitionWriter->writeBooleanArray(fieldName, data);
            }

            void PortableWriter::writeCharArray(const char *fieldName, const std::vector<char> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeCharArray(fieldName, data);
                return classDefinitionWriter->writeCharArray(fieldName, data);
            }

            void PortableWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShortArray(fieldName, data);
                return classDefinitionWriter->writeShortArray(fieldName, data);
            }

            void PortableWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeIntArray(fieldName, data);
                return classDefinitionWriter->writeIntArray(fieldName, data);
            }

            void PortableWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLongArray(fieldName, data);
                return classDefinitionWriter->writeLongArray(fieldName, data);
            }

            void PortableWriter::writeFloatArray(const char *fieldName, const std::vector<float> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloatArray(fieldName, data);
                return classDefinitionWriter->writeFloatArray(fieldName, data);
            }

            void PortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDoubleArray(fieldName, data);
                return classDefinitionWriter->writeDoubleArray(fieldName, data);
            }

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

            int32_t getHazelcastTypeId(const Portable *portable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }

            int32_t getHazelcastTypeId(const IdentifiedDataSerializable *identifiedDataSerializable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int32_t getHazelcastTypeId(const char *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            int32_t getHazelcastTypeId(const bool *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            }

            int32_t getHazelcastTypeId(const byte *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE;
            }

            int32_t getHazelcastTypeId(const int16_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT;
            }

            int32_t getHazelcastTypeId(const int32_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER;
            }

            int32_t getHazelcastTypeId(const int64_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG;
            }

            int32_t getHazelcastTypeId(const float *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT;
            }

            int32_t getHazelcastTypeId(const double *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE;
            }

            int32_t getHazelcastTypeId(const std::string *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
            }

            int32_t getHazelcastTypeId(const std::vector<char> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<bool> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<byte> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int16_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int32_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int64_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<float> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<double> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string *> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }

            int32_t getHazelcastTypeId(const void *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_VOID_POINTER;
            }

            int32_t getHazelcastTypeId(const HazelcastJsonValue *object) {
                return pimpl::SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
            }

            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), index(0), done(false) {

            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addIntField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addLongField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addUTFField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_UTF);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addBooleanField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addByteField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addCharField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addShortField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addByteArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addBooleanArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addCharArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addIntArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addLongArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addShortArrayField(const std::string &fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName,
                                                                             std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableArrayField(const std::string &fieldName,
                                                                                  std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY,
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
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf);
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
                    throw exception::HazelcastSerializationException("ClassDefinitionBuilder::check",
                                                                     "ClassDefinition is already built for " +
                                                                     util::IOUtil::to_string(classId));
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
                      version(version) {
            }

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
                dataOutput.writeInt(index);
                dataOutput.writeUTF(&fieldName);
                dataOutput.writeByte(type.getId());
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
            }

            void FieldDefinition::readData(pimpl::DataInput &dataInput) {
                index = dataInput.readInt();
                fieldName = *dataInput.readUTF();
                type.id = dataInput.readByte();
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
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
                   << " type: " << definition.type << " classId: " << definition.classId << " factoryId: "
                   << definition.factoryId << " version: " << definition.version;
                return os;
            }

            ObjectDataInput::ObjectDataInput(pimpl::DataInput &dataInput, pimpl::SerializerHolder &serializerHolder)
                    : dataInput(dataInput), serializerHolder(serializerHolder) {
            }

            void ObjectDataInput::readFully(std::vector<byte> &bytes) {
                dataInput.readFully(bytes);
            }

            int ObjectDataInput::skipBytes(int i) {
                return dataInput.skipBytes(i);
            }

            bool ObjectDataInput::readBoolean() {
                return dataInput.readBoolean();
            }

            byte ObjectDataInput::readByte() {
                return dataInput.readByte();
            }

            int16_t ObjectDataInput::readShort() {
                return dataInput.readShort();
            }

            char ObjectDataInput::readChar() {
                return dataInput.readChar();
            }

            int32_t ObjectDataInput::readInt() {
                return dataInput.readInt();
            }

            int64_t ObjectDataInput::readLong() {
                return dataInput.readLong();
            }

            float ObjectDataInput::readFloat() {
                return dataInput.readFloat();
            }

            double ObjectDataInput::readDouble() {
                return dataInput.readDouble();
            }

            std::unique_ptr<std::string> ObjectDataInput::readUTF() {
                return dataInput.readUTF();
            }

            pimpl::Data ObjectDataInput::readData() {
                return pimpl::Data(dataInput.readByteArray());
            }

            int ObjectDataInput::position() {
                return dataInput.position();
            }

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            }

            std::unique_ptr<std::vector<byte> > ObjectDataInput::readByteArray() {
                return dataInput.readByteArray();
            }

            std::unique_ptr<std::vector<bool> > ObjectDataInput::readBooleanArray() {
                return dataInput.readBooleanArray();
            }

            std::unique_ptr<std::vector<char> > ObjectDataInput::readCharArray() {
                return dataInput.readCharArray();
            }

            std::unique_ptr<std::vector<int32_t> > ObjectDataInput::readIntArray() {
                return dataInput.readIntArray();
            }

            std::unique_ptr<std::vector<int64_t> > ObjectDataInput::readLongArray() {
                return dataInput.readLongArray();
            }

            std::unique_ptr<std::vector<double> > ObjectDataInput::readDoubleArray() {
                return dataInput.readDoubleArray();
            }

            std::unique_ptr<std::vector<float> > ObjectDataInput::readFloatArray() {
                return dataInput.readFloatArray();
            }

            std::unique_ptr<std::vector<int16_t> > ObjectDataInput::readShortArray() {
                return dataInput.readShortArray();
            }

            std::unique_ptr<std::vector<std::string> > ObjectDataInput::readUTFArray() {
                return dataInput.readUTFArray();
            }

            std::unique_ptr<std::vector<std::string *> > ObjectDataInput::readUTFPointerArray() {
                return dataInput.readUTFPointerArray();
            }

            template<>
            std::vector<std::string> *ObjectDataInput::getBackwardCompatiblePointer(void *actualData,
                                                                                    const std::vector<std::string> *typePointer) const {
                std::unique_ptr<std::vector<std::string> > result(new std::vector<std::string>());
                typedef std::vector<std::string *> STRING_PONTER_ARRAY;
                std::vector<std::string *> *data = reinterpret_cast<std::vector<std::string *> *>(actualData);
                // it is guaranteed that the data will not be null
                for (STRING_PONTER_ARRAY::value_type value  : *data) {
                    if ((std::string *) NULL == value) {
                        result->push_back("");
                    } else {
                        result->push_back(*value);
                    }
                }
                return result.release();
            }

            template<>
            std::unique_ptr<HazelcastJsonValue>
            ObjectDataInput::readObjectInternal(int32_t typeId, const std::shared_ptr<SerializerBase> &serializer) {
                std::shared_ptr<StreamSerializer> streamSerializer = std::static_pointer_cast<StreamSerializer>(
                        serializer);

                return std::unique_ptr<HazelcastJsonValue>(
                        getBackwardCompatiblePointer<HazelcastJsonValue>(streamSerializer->read(*this),
                                                                         (HazelcastJsonValue *) NULL));
            }

            FieldType::FieldType() : id(0) {
            }

            FieldType::FieldType(int type) : id((byte) type) {
            }

            FieldType::FieldType(FieldType const &rhs) : id(rhs.id) {
            }

            const byte FieldType::getId() const {
                return id;
            }

            FieldType &FieldType::operator=(FieldType const &rhs) {
                this->id = rhs.id;
                return (*this);
            }

            bool FieldType::operator==(FieldType const &rhs) const {
                if (id != rhs.id) return false;
                return true;
            }

            bool FieldType::operator!=(FieldType const &rhs) const {
                if (id == rhs.id) return false;
                return true;
            }

            std::ostream &operator<<(std::ostream &os, const FieldType &type) {
                os << "FieldType{id: " << type.id << "}";
                return os;
            }

            SerializerBase::~SerializerBase() {
            }

            void SerializerBase::destroy() {
            }

            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput &dataOutput, pimpl::SerializerHolder *serializerHolder)
                    : dataOutput(&dataOutput), serializerHolder(serializerHolder), isEmpty(false) {

            }

            ObjectDataOutput::ObjectDataOutput()
                    : dataOutput(NULL), serializerHolder(NULL), isEmpty(true) {
            }

            std::unique_ptr<std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return std::unique_ptr<std::vector<byte> >((std::vector<byte> *) NULL);
                return dataOutput->toByteArray();
            }

            void ObjectDataOutput::write(const std::vector<byte> &bytes) {
                if (isEmpty) return;
                dataOutput->write(bytes);
            }

            void ObjectDataOutput::writeBoolean(bool i) {
                if (isEmpty) return;
                dataOutput->writeBoolean(i);
            }

            void ObjectDataOutput::writeByte(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeByte(i);
            }

            void ObjectDataOutput::writeBytes(const byte *bytes, size_t len) {
                if (isEmpty) return;
                dataOutput->writeBytes(bytes, len);
            }

            void ObjectDataOutput::writeShort(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeShort(v);
            }

            void ObjectDataOutput::writeChar(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeChar(i);
            }

            void ObjectDataOutput::writeInt(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeInt(v);
            }

            void ObjectDataOutput::writeLong(int64_t l) {
                if (isEmpty) return;
                dataOutput->writeLong(l);
            }

            void ObjectDataOutput::writeFloat(float x) {
                if (isEmpty) return;
                dataOutput->writeFloat(x);
            }

            void ObjectDataOutput::writeDouble(double v) {
                if (isEmpty) return;
                dataOutput->writeDouble(v);
            }

            void ObjectDataOutput::writeUTF(const std::string *str) {
                if (isEmpty) return;

                if (NULL == str) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    dataOutput->writeUTF(str);
                }
            }

            void ObjectDataOutput::writeByteArray(const std::vector<byte> *value) {
                if (isEmpty) return;
                dataOutput->writeByteArray(value);
            }

            void ObjectDataOutput::writeCharArray(const std::vector<char> *data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            }

            void ObjectDataOutput::writeBooleanArray(const std::vector<bool> *data) {
                if (isEmpty) return;
                dataOutput->writeBooleanArray(data);
            }

            void ObjectDataOutput::writeShortArray(const std::vector<int16_t> *data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            }

            void ObjectDataOutput::writeIntArray(const std::vector<int32_t> *data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            }

            void ObjectDataOutput::writeLongArray(const std::vector<int64_t> *data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            }

            void ObjectDataOutput::writeFloatArray(const std::vector<float> *data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            }

            void ObjectDataOutput::writeDoubleArray(const std::vector<double> *data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            }

            void ObjectDataOutput::writeUTFArray(const std::vector<std::string *> *strings) {
                if (isEmpty) return;

                int32_t len = NULL != strings ? (int32_t) strings->size() : util::Bits::NULL_ARRAY;

                writeInt(len);

                if (len > 0) {
                    for (std::vector<std::string *>::const_iterator it = strings->begin(); it != strings->end(); ++it) {
                        writeUTF(*it);
                    }
                }
            }

            void ObjectDataOutput::writeData(const pimpl::Data *data) {
                if (NULL == data || 0 == data->dataSize()) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    writeByteArray(&data->toByteArray());
                }

            }

            size_t ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(size_t newPos) {
                dataOutput->position(newPos);
            }

            pimpl::DataOutput *ObjectDataOutput::getDataOutput() const {
                return dataOutput;
            }

            template<>
            void ObjectDataOutput::writeInternal(const std::vector<std::string> *object,
                                                 std::shared_ptr<StreamSerializer> &streamSerializer) {
                std::vector<std::string> *stringVector = const_cast<std::vector<std::string> *>(object);
                std::unique_ptr<std::vector<std::string *> > result(new std::vector<std::string *>());
                for (std::vector<std::string>::iterator it = stringVector->begin(); it != stringVector->end(); ++it) {
                    result->push_back(&(*it));
                }

                streamSerializer->write(*this, result.get());
            }

            PortableReader::PortableReader(pimpl::PortableContext &context, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd, bool isDefaultReader)
                    : isDefaultReader(isDefaultReader) {
                if (isDefaultReader) {
                    defaultPortableReader.reset(new pimpl::DefaultPortableReader(context, input, cd));
                } else {
                    morphingPortableReader.reset(new pimpl::MorphingPortableReader(context, input, cd));
                }
            }

            PortableReader::PortableReader(const PortableReader &reader)
                    : isDefaultReader(reader.isDefaultReader),
                      defaultPortableReader(reader.defaultPortableReader.release()),
                      morphingPortableReader(reader.morphingPortableReader.release()) {

            }

            PortableReader &PortableReader::operator=(const PortableReader &reader) {
                this->isDefaultReader = reader.isDefaultReader;
                this->defaultPortableReader.reset(reader.defaultPortableReader.release());
                this->morphingPortableReader.reset(reader.morphingPortableReader.release());
                return *this;
            }

            int32_t PortableReader::readInt(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readInt(fieldName);
                return morphingPortableReader->readInt(fieldName);
            }

            int64_t PortableReader::readLong(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLong(fieldName);
                return morphingPortableReader->readLong(fieldName);
            }

            bool PortableReader::readBoolean(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBoolean(fieldName);
                return morphingPortableReader->readBoolean(fieldName);
            }

            byte PortableReader::readByte(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByte(fieldName);
                return morphingPortableReader->readByte(fieldName);
            }

            char PortableReader::readChar(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readChar(fieldName);
                return morphingPortableReader->readChar(fieldName);
            }

            double PortableReader::readDouble(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDouble(fieldName);
                return morphingPortableReader->readDouble(fieldName);
            }

            float PortableReader::readFloat(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloat(fieldName);
                return morphingPortableReader->readFloat(fieldName);
            }

            int16_t PortableReader::readShort(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShort(fieldName);
                return morphingPortableReader->readShort(fieldName);
            }

            std::unique_ptr<std::string> PortableReader::readUTF(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readUTF(fieldName);
                return morphingPortableReader->readUTF(fieldName);
            }

            std::unique_ptr<std::vector<byte> > PortableReader::readByteArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByteArray(fieldName);
                return morphingPortableReader->readByteArray(fieldName);
            }


            std::unique_ptr<std::vector<bool> > PortableReader::readBooleanArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBooleanArray(fieldName);
                return morphingPortableReader->readBooleanArray(fieldName);
            }

            std::unique_ptr<std::vector<char> > PortableReader::readCharArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readCharArray(fieldName);
                return morphingPortableReader->readCharArray(fieldName);
            }

            std::unique_ptr<std::vector<int32_t> > PortableReader::readIntArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readIntArray(fieldName);
                return morphingPortableReader->readIntArray(fieldName);
            }

            std::unique_ptr<std::vector<int64_t> > PortableReader::readLongArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLongArray(fieldName);
                return morphingPortableReader->readLongArray(fieldName);
            }

            std::unique_ptr<std::vector<double> > PortableReader::readDoubleArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDoubleArray(fieldName);
                return morphingPortableReader->readDoubleArray(fieldName);
            }

            std::unique_ptr<std::vector<float> > PortableReader::readFloatArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloatArray(fieldName);
                return morphingPortableReader->readFloatArray(fieldName);
            }

            std::unique_ptr<std::vector<int16_t> > PortableReader::readShortArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShortArray(fieldName);
                return morphingPortableReader->readShortArray(fieldName);
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

            const FieldDefinition &ClassDefinition::getField(const char *name) const {
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return fieldDefinitionsMap.find(name)->second;
                }
                char msg[200];
                util::hz_snprintf(msg, 200, "Field (%s) does not exist", NULL != name ? name : "");
                throw exception::IllegalArgumentException("ClassDefinition::getField", msg);
            }

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
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

            void ClassDefinition::setVersionIfNotSet(int version) {
                if (getVersion() < 0) {
                    this->version = version;
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
                dataOutput.writeInt(version);
                dataOutput.writeShort(fieldDefinitionsMap.size());
                for (std::map<std::string, FieldDefinition>::iterator it = fieldDefinitionsMap.begin();
                     it != fieldDefinitionsMap.end(); ++it) {
                    it->second.writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(pimpl::DataInput &dataInput) {
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
                version = dataInput.readInt();
                int size = dataInput.readShort();
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

                for (std::map<std::string, FieldDefinition>::const_iterator it = definition.fieldDefinitionsMap.begin();
                     it != definition.fieldDefinitionsMap.end(); ++it) {
                    os << it->second;
                }
                os << "} }";
                return os;
            }

            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder(builder), context(portableContext) {
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                void ClassDefinitionWriter::writeInt(const char *fieldName, int32_t value) {
                    builder.addIntField(fieldName);
                }

                void ClassDefinitionWriter::writeLong(const char *fieldName, int64_t value) {
                    builder.addLongField(fieldName);
                }

                void ClassDefinitionWriter::writeBoolean(const char *fieldName, bool value) {

                    builder.addBooleanField(fieldName);
                }

                void ClassDefinitionWriter::writeByte(const char *fieldName, byte value) {
                    builder.addByteField(fieldName);
                }

                void ClassDefinitionWriter::writeChar(const char *fieldName, int32_t value) {
                    builder.addCharField(fieldName);
                }

                void ClassDefinitionWriter::writeDouble(const char *fieldName, double value) {

                    builder.addDoubleField(fieldName);
                }

                void ClassDefinitionWriter::writeFloat(const char *fieldName, float value) {
                    builder.addFloatField(fieldName);
                }

                void ClassDefinitionWriter::writeShort(const char *fieldName, int16_t value) {
                    builder.addShortField(fieldName);
                }

                void ClassDefinitionWriter::writeUTF(const char *fieldName, const std::string *value) {

                    builder.addUTFField(fieldName);
                }

                void ClassDefinitionWriter::writeByteArray(const char *fieldName, const std::vector<byte> *values) {
                    builder.addByteArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *values) {
                    builder.addBooleanArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeCharArray(const char *fieldName, const std::vector<char> *values) {
                    builder.addCharArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *values) {
                    builder.addIntArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *values) {
                    builder.addLongArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *values) {
                    builder.addDoubleArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeFloatArray(const char *fieldName, const std::vector<float> *values) {
                    builder.addFloatArrayField(fieldName);
                }

                void ClassDefinitionWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *values) {
                    builder.addShortArrayField(fieldName);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
                }

                void ClassDefinitionWriter::end() {

                }

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::createNestedClassDef(const Portable &portable) {
                    int version = pimpl::PortableVersionHelper::getVersion(&portable, context.getVersion());
                    ClassDefinitionBuilder definitionBuilder(portable.getFactoryId(), portable.getClassId(), version);

                    ClassDefinitionWriter nestedWriter(context, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    portable.writePortable(portableWriter);
                    return context.registerClassDefinition(definitionBuilder.build());
                }

                size_t const DataOutput::DEFAULT_SIZE = 4 * 1024;

                DataOutput::DataOutput()
                        : outputStream(new std::vector<byte>()) {
                    outputStream->reserve(DEFAULT_SIZE);
                }


                DataOutput::~DataOutput() {
                }

                DataOutput::DataOutput(DataOutput const &rhs) {
                    //private
                }

                DataOutput &DataOutput::operator=(DataOutput const &rhs) {
                    //private
                    return *this;
                }

                std::unique_ptr<std::vector<byte> > DataOutput::toByteArray() {
                    std::unique_ptr<std::vector<byte> > byteArrayPtr(new std::vector<byte>(*outputStream));
                    return byteArrayPtr;
                }

                void DataOutput::write(const std::vector<byte> &bytes) {
                    outputStream->insert(outputStream->end(), bytes.begin(), bytes.end());
                }

                void DataOutput::writeBoolean(bool i) {
                    writeByte((byte) i);
                }

                void DataOutput::writeByte(int index, int32_t i) {
                    (*outputStream)[index] = byte(0xff & i);
                }

                void DataOutput::writeByte(int32_t i) {
                    outputStream->push_back(byte(0xff & i));
                }

                void DataOutput::writeShort(int32_t v) {
                    int16_t value = (int16_t) v;
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                void DataOutput::writeChar(int32_t i) {
                    writeByte((byte) (i >> 8));
                    writeByte((byte) i);
                }

                void DataOutput::writeInt(int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                void DataOutput::writeLong(int64_t l) {
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, target);
                    outputStream->insert(outputStream->end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                void DataOutput::writeFloat(float x) {
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    writeInt(u.i);
                }

                void DataOutput::writeDouble(double v) {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    writeLong(u.l);
                }

                void DataOutput::writeUTF(const std::string *str) {
                    int32_t len = util::Bits::NULL_ARRAY;
                    if (str) {
                        len = util::UTFUtil::isValidUTF8(*str);
                        if (len < 0) {
                            throw (exception::ExceptionBuilder<exception::UTFDataFormatException>(
                                    "DataOutput::writeUTF")
                                    << "String \"" << (*str) << "\" is not UTF-8 formatted !!!").build();
                        }
                    }

                    writeInt(len);
                    if (len > 0) {
                        outputStream->insert(outputStream->end(), str->begin(), str->end());
                    }
                }

                void DataOutput::writeInt(int index, int32_t v) {
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, &result);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index++] = *(target++);
                    (*outputStream)[index] = *target;
                }

                void DataOutput::writeBytes(const byte *bytes, size_t len) {
                    outputStream->insert(outputStream->end(), bytes, bytes + len);
                }

                void DataOutput::writeByteArray(const std::vector<byte> *data) {
                    writeArray<byte>(data);
                }

                void DataOutput::writeCharArray(const std::vector<char> *data) {
                    writeArray<char>(data);
                }

                void DataOutput::writeBooleanArray(const std::vector<bool> *data) {
                    writeArray<bool>(data);
                }

                void DataOutput::writeShortArray(const std::vector<int16_t> *data) {
                    writeArray<int16_t>(data);
                }

                void DataOutput::writeIntArray(const std::vector<int32_t> *data) {
                    writeArray<int32_t>(data);
                }

                void DataOutput::writeLongArray(const std::vector<int64_t> *data) {
                    writeArray<int64_t>(data);
                }

                void DataOutput::writeFloatArray(const std::vector<float> *data) {
                    writeArray<float>(data);
                }

                void DataOutput::writeDoubleArray(const std::vector<double> *data) {
                    writeArray<double>(data);
                }

                void DataOutput::writeUTFArray(const std::vector<std::string> *data) {
                    writeArray<std::string>(data);
                }

                void DataOutput::writeZeroBytes(int numberOfBytes) {
                    for (int k = 0; k < numberOfBytes; k++) {
                        writeByte(0);
                    }
                }

                size_t DataOutput::position() {
                    return outputStream->size();
                }

                void DataOutput::position(size_t newPos) {
                    if (outputStream->size() < newPos)
                        outputStream->resize(newPos, 0);
                }

                int DataOutput::getUTF8CharCount(const std::string &str) {
                    int size = 0;
                    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
                        // Any additional byte for an UTF character has a bit mask of 10xxxxxx
                        size += (*it & 0xC0) != 0x80;
                    }

                    return size;
                }

                template<>
                void DataOutput::write(const byte &value) {
                    writeByte(value);
                }

                template<>
                void DataOutput::write(const char &value) {
                    writeChar(value);
                }

                template<>
                void DataOutput::write(const bool &value) {
                    writeBoolean(value);
                }

                template<>
                void DataOutput::write(const int16_t &value) {
                    writeShort(value);
                }

                template<>
                void DataOutput::write(const int32_t &value) {
                    writeInt(value);
                }

                template<>
                void DataOutput::write(const int64_t &value) {
                    writeLong(value);
                }

                template<>
                void DataOutput::write(const float &value) {
                    writeFloat(value);
                }

                template<>
                void DataOutput::write(const double &value) {
                    writeDouble(value);
                }

                template<>
                void DataOutput::write(const std::string &value) {
                    writeUTF(&value);
                }

                ObjectType::ObjectType() : typeId(0), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << type.typeId << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {
                }

                DataSerializer::~DataSerializer() {
                }

                void DataSerializer::checkIfIdentifiedDataSerializable(ObjectDataInput &in) const {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read",
                                                                         " DataSerializable is not identified");
                    }
                }

                int32_t DataSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DATA;
                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable *dataSerializable) {
                    out.writeBoolean(true);
                    out.writeInt(dataSerializable->getFactoryId());
                    out.writeInt(dataSerializable->getClassId());
                    dataSerializable->writeData(out);
                }

                void DataSerializer::write(ObjectDataOutput &out, const void *object) {
                    // should not be called
                    assert(0);
                }

                void *DataSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }

                PortableContext::PortableContext(const SerializationConfig &serializationConf) :
                        serializationConfig(serializationConf),
                        serializerHolder(serializationConf.getGlobalSerializer()) {
                }

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
                    in.readInt();

                    // field count
                    int fieldCount = in.readInt();
                    int offset = in.position();
                    for (int i = 0; i < fieldCount; i++) {
                        in.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                        int pos = in.readInt();
                        in.position(pos);

                        short len = in.readShort();
                        std::vector<byte> chars(len);
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(in.readByte());
                        std::string name((char *) &(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        int fieldVersion = version;
                        if (type == FieldTypes::TYPE_PORTABLE) {
                            // is null
                            if (in.readBoolean()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.readInt();
                            fieldClassId = in.readInt();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldTypes::TYPE_PORTABLE_ARRAY) {
                            int k = in.readInt();
                            if (k > 0) {
                                fieldFactoryId = in.readInt();
                                fieldClassId = in.readInt();

                                int p = in.readInt();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                fieldVersion = in.readInt();
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

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupOrRegisterClassDefinition(const Portable &portable) {
                    int portableVersion = PortableVersionHelper::getVersion(&portable,
                                                                            serializationConfig.getPortableVersion());
                    std::shared_ptr<ClassDefinition> cd = lookupClassDefinition(portable.getFactoryId(),
                                                                                portable.getClassId(), portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(portable.getFactoryId(), portable.getClassId(),
                                                                      portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        portable.writePortable(portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                int PortableContext::getVersion() {
                    return serializationConfig.getPortableVersion();
                }

                SerializerHolder &PortableContext::getSerializerHolder() {
                    return serializerHolder;
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
                        : portableContext(serializationConfig),
                          serializationConfig(serializationConfig) {
                    registerConstantSerializers();

                    std::vector<std::shared_ptr<SerializerBase> > const &serializers = serializationConfig.getSerializers();
                    for (std::vector<std::shared_ptr<SerializerBase> >::const_iterator it = serializers.begin();
                         it < serializers.end(); ++it) {
                        registerSerializer(std::static_pointer_cast<StreamSerializer>(*it));
                    }
                }

                SerializerHolder &SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }

                DefaultPortableWriter::DefaultPortableWriter(PortableContext &portableContext,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw(false), serializerHolder(portableContext.getSerializerHolder()),
                          dataOutput(*output.getDataOutput()), objectDataOutput(output), begin(dataOutput.position()),
                          cd(cd) {
                    // room for final offset
                    dataOutput.writeZeroBytes(4);

                    objectDataOutput.writeInt(cd->getFieldCount());

                    offset = dataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    dataOutput.writeZeroBytes(fieldIndexesLength);
                }

                void DefaultPortableWriter::writeInt(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    dataOutput.writeInt(value);
                }

                void DefaultPortableWriter::writeLong(const char *fieldName, int64_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    dataOutput.writeLong(value);
                }

                void DefaultPortableWriter::writeBoolean(const char *fieldName, bool value) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    dataOutput.writeBoolean(value);
                }

                void DefaultPortableWriter::writeByte(const char *fieldName, byte value) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    dataOutput.writeByte(value);
                }

                void DefaultPortableWriter::writeChar(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    dataOutput.writeChar(value);
                }

                void DefaultPortableWriter::writeDouble(const char *fieldName, double value) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    dataOutput.writeDouble(value);
                }

                void DefaultPortableWriter::writeFloat(const char *fieldName, float value) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    dataOutput.writeFloat(value);
                }

                void DefaultPortableWriter::writeShort(const char *fieldName, int32_t value) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    dataOutput.writeShort(value);
                }

                void DefaultPortableWriter::writeUTF(const char *fieldName, const std::string *value) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    dataOutput.writeUTF(value);
                }

                void DefaultPortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    dataOutput.writeByteArray(bytes);
                }

                void DefaultPortableWriter::writeBooleanArray(const char *fieldName, const std::vector<bool> *bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    dataOutput.writeBooleanArray(bytes);
                }

                void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    dataOutput.writeCharArray(data);
                }

                void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<int16_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    dataOutput.writeShortArray(data);
                }

                void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int32_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    dataOutput.writeIntArray(data);
                }

                void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<int64_t> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    dataOutput.writeLongArray(data);
                }

                void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    dataOutput.writeFloatArray(data);
                }

                void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double> *data) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    dataOutput.writeDoubleArray(data);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const char *fieldName, FieldType fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                         "Cannot write Portable fields after getRawDataOutput() is called!");
                    }

                    try {
                        FieldDefinition const &fd = cd->getField(fieldName);

                        if (writtenFields.count(fieldName) != 0) {
                            throw exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                             "Field '" + std::string(fieldName) +
                                                                             "' has already been written!");
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = dataOutput.position();
                        int32_t index = fd.getIndex();
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                        size_t nameLen = strlen(fieldName);
                        dataOutput.writeShort(nameLen);
                        dataOutput.writeBytes((byte *) fieldName, nameLen);
                        dataOutput.writeByte(fieldType.getId());

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}. Error:";
                        error << iae.what();

                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", error.str());
                    }

                }


                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = dataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        dataOutput.writeInt((int32_t) (offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t) pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt((int32_t) begin, (int32_t) dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable &p) {
                    std::shared_ptr<PortableSerializer> serializer = std::static_pointer_cast<PortableSerializer>(
                            serializerHolder.serializerFor(SerializationConstants::CONSTANT_TYPE_PORTABLE));
                    serializer->writeInternal(objectDataOutput, &p);
                }


                void
                DefaultPortableWriter::checkPortableAttributes(const FieldDefinition &fd, const Portable &portable) {
                    if (fd.getFactoryId() != portable.getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << portable.getFactoryId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                    if (fd.getClassId() != portable.getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << portable.getClassId();
                        throw exception::HazelcastSerializationException(
                                "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                }

                bool SerializationService::registerSerializer(std::shared_ptr<StreamSerializer> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        return type;
                    }

                    type.typeId = data->getType();

                    // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                    // let usage of static member.
                    DataInput dataInput(data->toByteArray(), 4);

                    ObjectDataInput objectDataInput(dataInput, getSerializerHolder());

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                        SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        int32_t objectTypeId = objectDataInput.readInt();
                        assert(type.typeId == objectTypeId);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = objectDataInput.readBoolean();
                            if (!identified) {
                                throw exception::HazelcastSerializationException("SerializationService::getObjectType",
                                                                                 " DataSerializable is not identified");
                            }
                        }

                        type.factoryId = objectDataInput.readInt();
                        type.classId = objectDataInput.readInt();
                    }

                    return type;
                }

                void SerializationService::registerConstantSerializers() {
                    registerSerializer(std::shared_ptr<StreamSerializer>(new NullSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DataSerializer(serializationConfig)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new PortableSerializer(portableContext)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new HazelcastJsonValueSerializer()));
                    //primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ByteSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::BooleanSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::CharSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ShortSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::IntegerSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::LongSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::FloatSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::DoubleSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new TheByteArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new BooleanArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new CharArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new ShortArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new IntegerArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new LongArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new FloatArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DoubleArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new StringArraySerializer()));
                }

                void SerializationService::dispose() {
                    getSerializerHolder().dispose();
                }

                template<>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    const std::shared_ptr<Data> data = object->getData();
                    if ((Data *) NULL == data.get()) {
                        return Data();
                    }

                    return Data(*data);
                }

                //first 4 byte is partition hash code and next last 4 byte is type id
                unsigned int Data::PARTITION_HASH_OFFSET = 0;

                unsigned int Data::TYPE_OFFSET = Data::PARTITION_HASH_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OFFSET = Data::TYPE_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OVERHEAD = Data::DATA_OFFSET;

                Data::Data() : cachedHashValue(-1) {
                }

                Data::Data(std::unique_ptr<std::vector<byte> > &buffer) : Data::Data(std::move(buffer)) {}

                Data::Data(std::unique_ptr<std::vector<byte> > &&buffer) : data(std::move(buffer)),
                                                                           cachedHashValue(-1) {
                    if (data.get()) {
                        size_t size = data->size();
                        if (size > 0 && size < Data::DATA_OVERHEAD) {
                            throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Data::setBuffer")
                                    << "Provided buffer should be either empty or should contain more than "
                                    << Data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size).build();
                        }

                        cachedHashValue = calculateHash();
                    }
                }

                size_t Data::dataSize() const {
                    return (size_t) std::max<int>((int) totalSize() - (int) Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data.get() != 0 ? data->size() : 0;
                }

                int Data::getPartitionHash() const {
                    return cachedHashValue;
                }

                bool Data::hasPartitionHash() const {
                    if (data.get() == NULL) {
                        return false;
                    }
                    return data->size() >= Data::DATA_OVERHEAD &&
                           *reinterpret_cast<int *>(&((*data)[PARTITION_HASH_OFFSET])) != 0;
                }

                std::vector<byte> &Data::toByteArray() const {
                    return *data;
                }

                int32_t Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return util::Bits::readIntB(*data, Data::TYPE_OFFSET);
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
                        return util::Bits::readIntB(*data, Data::PARTITION_HASH_OFFSET);
                    }

                    return util::MurmurHash3_x86_32((void *) &((*data)[Data::DATA_OFFSET]), (int) size);
                }

                bool Data::operator<(const Data &rhs) const {
                    return cachedHashValue < rhs.cachedHashValue;
                }

                DataInput::DataInput(const std::vector<byte> &buf) : buffer(buf), pos(0) {
                }

                DataInput::DataInput(const std::vector<byte> &buf, int offset)
                        : buffer(buf), pos(offset) {
                }

                DataInput::~DataInput() {
                }

                void DataInput::readFully(std::vector<byte> &bytes) {
                    size_t length = bytes.size();
                    checkAvailable(length);
                    memcpy(&(bytes[0]), &(buffer[pos]), length);
                    pos += length;
                }

                int DataInput::skipBytes(int i) {
                    checkAvailable(i);
                    pos += i;
                    return i;
                }

                bool DataInput::readBoolean() {
                    checkAvailable(1);
                    return readBooleanUnchecked();
                }

                bool DataInput::readBooleanUnchecked() { return readByteUnchecked() != 0; }

                byte DataInput::readByte() {
                    checkAvailable(1);
                    return readByteUnchecked();
                }

                byte DataInput::readByteUnchecked() { return buffer[pos++]; }

                int16_t DataInput::readShort() {
                    checkAvailable(util::Bits::SHORT_SIZE_IN_BYTES);
                    return readShortUnchecked();
                }

                int16_t DataInput::readShortUnchecked() {
                    int16_t result;
                    util::Bits::bigEndianToNative2(&buffer[pos], &result);
                    pos += util::Bits::SHORT_SIZE_IN_BYTES;
                    return result;
                }

                char DataInput::readChar() {
                    checkAvailable(util::Bits::CHAR_SIZE_IN_BYTES);
                    return readCharUnchecked();
                }

                char DataInput::readCharUnchecked() {
                    // skip the first byte
                    byte b = buffer[pos + 1];
                    pos += util::Bits::CHAR_SIZE_IN_BYTES;
                    return b;
                }

                int32_t DataInput::readInt() {
                    checkAvailable(util::Bits::INT_SIZE_IN_BYTES);
                    return readIntUnchecked();
                }

                int32_t DataInput::readIntUnchecked() {
                    int32_t result;
                    util::Bits::bigEndianToNative4(&buffer[pos], &result);
                    pos += util::Bits::INT_SIZE_IN_BYTES;
                    return result;
                }

                int64_t DataInput::readLong() {
                    checkAvailable(util::Bits::LONG_SIZE_IN_BYTES);
                    return readLongUnchecked();
                }

                int64_t DataInput::readLongUnchecked() {
                    int64_t result;
                    util::Bits::bigEndianToNative8(&buffer[pos], &result);
                    pos += util::Bits::LONG_SIZE_IN_BYTES;
                    return result;
                }

                float DataInput::readFloat() {
                    checkAvailable(util::Bits::FLOAT_SIZE_IN_BYTES);
                    return readFloatUnchecked();
                }

                float DataInput::readFloatUnchecked() {
                    union {
                        int32_t i;
                        float f;
                    } u;
                    u.i = readIntUnchecked();
                    return u.f;
                }

                double DataInput::readDouble() {
                    checkAvailable(util::Bits::DOUBLE_SIZE_IN_BYTES);
                    return readDoubleUnchecked();
                }

                double DataInput::readDoubleUnchecked() {
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.l = readLongUnchecked();
                    return u.d;
                }

                std::unique_ptr<std::string> DataInput::readUTF() {
                    int32_t charCount = readInt();
                    if (util::Bits::NULL_ARRAY == charCount) {
                        return std::unique_ptr<std::string>();
                    } else {
                        utfBuffer.clear();
                        utfBuffer.reserve((size_t) MAX_UTF_CHAR_SIZE * charCount);
                        byte b;
                        for (int i = 0; i < charCount; ++i) {
                            b = readByte();
                            util::UTFUtil::readUTF8Char(*this, b, utfBuffer);
                        }

                        return std::unique_ptr<std::string>(new std::string(utfBuffer.begin(), utfBuffer.end()));
                    }
                }

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int position) {
                    if (position > pos) {
                        checkAvailable((size_t) (position - pos));
                    }
                    pos = position;
                }
                //private functions

                std::unique_ptr<std::vector<byte> > DataInput::readByteArray() {
                    return readArray<byte>();
                }

                std::unique_ptr<std::vector<bool> > DataInput::readBooleanArray() {
                    return readArray<bool>();
                }

                std::unique_ptr<std::vector<char> > DataInput::readCharArray() {
                    return readArray<char>();
                }

                std::unique_ptr<std::vector<int32_t> > DataInput::readIntArray() {
                    return readArray<int32_t>();
                }

                std::unique_ptr<std::vector<int64_t> > DataInput::readLongArray() {
                    return readArray<int64_t>();
                }

                std::unique_ptr<std::vector<double> > DataInput::readDoubleArray() {
                    return readArray<double>();
                }

                std::unique_ptr<std::vector<float> > DataInput::readFloatArray() {
                    return readArray<float>();
                }

                std::unique_ptr<std::vector<int16_t> > DataInput::readShortArray() {
                    return readArray<int16_t>();
                }

                std::unique_ptr<std::vector<std::string> > DataInput::readUTFArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string> >();
                    }

                    std::unique_ptr<std::vector<std::string> > values(
                            new std::vector<std::string>());
                    for (int32_t i = 0; i < len; ++i) {
                        std::unique_ptr<std::string> value = readUTF();
                        // handle null pointer possibility
                        if ((std::string *) NULL == value.get()) {
                            values->push_back(std::string(""));
                        } else {
                            values->push_back(*value);
                        }
                    }
                    return values;
                }

                std::unique_ptr<std::vector<std::string *> > DataInput::readUTFPointerArray() {
                    int32_t len = readInt();
                    if (util::Bits::NULL_ARRAY == len) {
                        return std::unique_ptr<std::vector<std::string *> >();
                    }

                    std::unique_ptr<std::vector<std::string *> > values(
                            new std::vector<std::string *>());
                    try {
                        for (int32_t i = 0; i < len; ++i) {
                            values->push_back(readUTF().release());
                        }
                    } catch (exception::IException &) {
                        // clean resources to avoid any leaks
                        typedef std::vector<std::string *> STRING_ARRAY;
                        for (STRING_ARRAY::value_type value  : *values) {
                            delete value;
                        }
                        throw;
                    }
                    return values;
                }

                void DataInput::checkAvailable(size_t requestedLength) {
                    size_t available = buffer.size() - pos;

                    if (requestedLength > available) {
                        char msg[100];
                        util::hz_snprintf(msg, 100,
                                          "Not enough bytes in internal buffer. Available:%lu bytes but needed %lu bytes",
                                          (unsigned long) available, (unsigned long) requestedLength);
                        throw exception::IOException("DataInput::checkBoundary", msg);
                    }
                }

                template<>
                byte DataInput::read() {
                    return readByteUnchecked();
                }

                template<>
                char DataInput::read() {
                    return readCharUnchecked();
                }

                template<>
                bool DataInput::read() {
                    return readBooleanUnchecked();
                }

                template<>
                int16_t DataInput::read() {
                    return readShortUnchecked();
                }

                template<>
                int32_t DataInput::read() {
                    return readIntUnchecked();
                }

                template<>
                int64_t DataInput::read() {
                    return readLongUnchecked();
                }

                template<>
                float DataInput::read() {
                    return readFloatUnchecked();
                }

                template<>
                double DataInput::read() {
                    return readDoubleUnchecked();
                }

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factoryId(factoryId), portableContext(portableContext) {
                }

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
                        throw exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion",
                                                                  error.str());
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

                DefaultPortableReader::DefaultPortableReader(PortableContext &portableContext,
                                                             ObjectDataInput &input,
                                                             std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }

                PortableReaderBase::PortableReaderBase(PortableContext &portableContext, ObjectDataInput &input,
                                                       std::shared_ptr<ClassDefinition> cd)
                        : cd(cd), dataInput(input), serializerHolder(portableContext.getSerializerHolder()),
                          raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.readInt();
                        // field count
                        fieldCount = input.readInt();
                    } catch (exception::IException &e) {
                        throw exception::HazelcastSerializationException(
                                "[PortableReaderBase::PortableReaderBase]", e.what());
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::hz_snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount,
                                          cd->getFieldCount());
                        throw new exception::IllegalStateException("[PortableReaderBase::PortableReaderBase]",
                                                                   msg);
                    }
                    this->offset = input.position();
                }

                PortableReaderBase::~PortableReaderBase() {

                }

                int32_t PortableReaderBase::readInt(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    return dataInput.readInt();
                }

                int64_t PortableReaderBase::readLong(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    return dataInput.readLong();
                }

                bool PortableReaderBase::readBoolean(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    return dataInput.readBoolean();
                }

                hazelcast::byte PortableReaderBase::readByte(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    return dataInput.readByte();
                }

                char PortableReaderBase::readChar(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    return dataInput.readChar();
                }

                double PortableReaderBase::readDouble(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    return dataInput.readDouble();
                }

                float PortableReaderBase::readFloat(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    return dataInput.readFloat();
                }

                int16_t PortableReaderBase::readShort(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    return dataInput.readShort();
                }

                std::unique_ptr<std::string> PortableReaderBase::readUTF(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    return dataInput.readUTF();
                }

                std::unique_ptr<std::vector<byte> > PortableReaderBase::readByteArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    return dataInput.readByteArray();
                }

                std::unique_ptr<std::vector<bool> > PortableReaderBase::readBooleanArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                    return dataInput.readBooleanArray();
                }

                std::unique_ptr<std::vector<char> > PortableReaderBase::readCharArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    return dataInput.readCharArray();
                }

                std::unique_ptr<std::vector<int32_t> > PortableReaderBase::readIntArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    return dataInput.readIntArray();
                }

                std::unique_ptr<std::vector<int64_t> > PortableReaderBase::readLongArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    return dataInput.readLongArray();
                }

                std::unique_ptr<std::vector<double> > PortableReaderBase::readDoubleArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    return dataInput.readDoubleArray();
                }

                std::unique_ptr<std::vector<float> > PortableReaderBase::readFloatArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    return dataInput.readFloatArray();
                }

                std::unique_ptr<std::vector<int16_t> > PortableReaderBase::readShortArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    return dataInput.readShortArray();
                }

                void PortableReaderBase::setPosition(char const *fieldName, FieldType const &fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int PortableReaderBase::readPosition(const char *fieldName, FieldType const &fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Cannot read Portable fields after getRawDataInput() is called!");
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Don't have a field named " +
                                                                         std::string(fieldName));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                         "Field type did not matched for " +
                                                                         std::string(fieldName));
                    }

                    dataInput.position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput.readInt();

                    dataInput.position(pos);
                    int16_t len = dataInput.readShort();

                    // name + len + type
                    return pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1;
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput.readInt();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput.position(finalPosition);
                }

                void
                PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int32_t factoryId, int32_t classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(),
                                          factoryId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(),
                                          classId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ",
                                                                         std::string(msg));
                    }
                }

                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeInt(*(static_cast<const int32_t *>(object)));
                }

                void *IntegerSerializer::read(ObjectDataInput &in) {
                    return new int32_t(in.readInt());
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByte(*(static_cast<const byte *>(object)));
                }

                void *ByteSerializer::read(ObjectDataInput &in) {
                    return new byte(in.readByte());
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBoolean(*(static_cast<const bool *>(object)));
                }

                void *BooleanSerializer::read(ObjectDataInput &in) {
                    return new bool(in.readBoolean());
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeChar(*(static_cast<const char *>(object)));
                }

                void *CharSerializer::read(ObjectDataInput &in) {
                    return new char(in.readChar());
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShort(*(static_cast<const int16_t *>(object)));
                }

                void *ShortSerializer::read(ObjectDataInput &in) {
                    return new int16_t(in.readShort());
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLong(*(static_cast<const int64_t *>(object)));
                }

                void *LongSerializer::read(ObjectDataInput &in) {
                    return new int64_t(in.readLong());;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloat(*(static_cast<const float *>(object)));
                }

                void *FloatSerializer::read(ObjectDataInput &in) {
                    return new float(in.readFloat());
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDouble(*(static_cast<const double *>(object)));
                }

                void *DoubleSerializer::read(ObjectDataInput &in) {
                    return new double(in.readDouble());
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(static_cast<const std::string *>(object));
                }

                void *StringSerializer::read(ObjectDataInput &in) {
                    return in.readUTF().release();
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const void *object) {
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByteArray(static_cast<const std::vector<byte> *>(object));
                }

                void *TheByteArraySerializer::read(ObjectDataInput &in) {
                    return in.readByteArray().release();
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBooleanArray(static_cast<const std::vector<bool> *>(object));
                }

                void *BooleanArraySerializer::read(ObjectDataInput &in) {
                    return in.readBooleanArray().release();
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeCharArray(static_cast<const std::vector<char> *>(object));
                }

                void *CharArraySerializer::read(ObjectDataInput &in) {
                    return in.readCharArray().release();
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShortArray(static_cast<const std::vector<int16_t> *>(object));
                }

                void *ShortArraySerializer::read(ObjectDataInput &in) {
                    return in.readShortArray().release();
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeIntArray(static_cast<const std::vector<int32_t> *>(object));
                }

                void *IntegerArraySerializer::read(ObjectDataInput &in) {
                    return in.readIntArray().release();
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLongArray(static_cast<const std::vector<int64_t> *>(object));
                }

                void *LongArraySerializer::read(ObjectDataInput &in) {
                    return in.readLongArray().release();
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloatArray(static_cast<const std::vector<float> *>(object));
                }

                void *FloatArraySerializer::read(ObjectDataInput &in) {
                    return in.readFloatArray().release();
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDoubleArray(static_cast<const std::vector<double> *>(object));
                }

                void *DoubleArraySerializer::read(ObjectDataInput &in) {
                    return in.readDoubleArray().release();
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTFArray(static_cast<const std::vector<std::string *> *>(object));
                }

                void *StringArraySerializer::read(ObjectDataInput &in) {
                    return in.readUTFPointerArray().release();
                }

                int32_t HazelcastJsonValueSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }

                void HazelcastJsonValueSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(&(static_cast<const HazelcastJsonValue *>(object)->toString()));
                }

                void *HazelcastJsonValueSerializer::read(ObjectDataInput &in) {
                    return new HazelcastJsonValue(*in.readUTF());
                }

                MorphingPortableReader::MorphingPortableReader(PortableContext &portableContext, ObjectDataInput &input,
                                                               std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableContext, input, cd) {
                }

                int32_t MorphingPortableReader::readInt(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int64_t MorphingPortableReader::readLong(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }

                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return (float) PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return (float) PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return (float) PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return (float) PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                int16_t MorphingPortableReader::readShort(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                         "IncompatibleClassChangeError");
                    }
                }

                std::unique_ptr<std::string> MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::string>(new std::string(""));
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::unique_ptr<std::vector<byte> > MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<byte> >(new std::vector<byte>(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::unique_ptr<std::vector<char> > MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<char> >(new std::vector<char>(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::unique_ptr<std::vector<int32_t> > MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int32_t> >(new std::vector<int32_t>(1, 0));
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::unique_ptr<std::vector<int64_t> > MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int64_t> >(new std::vector<int64_t>(1, 0));
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::unique_ptr<std::vector<double> > MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<double> >(new std::vector<double>(1, 0));
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::unique_ptr<std::vector<float> > MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<float> >(new std::vector<float>(1, 0));
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::unique_ptr<std::vector<int16_t> > MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int16_t> >(new std::vector<int16_t>(1, 0));
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }

                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            throw exception::IllegalArgumentException("PortableVersionHelper:getVersion",
                                                                      "Version cannot be negative!");
                        }
                    }
                    return version;
                }

                PortableSerializer::PortableSerializer(PortableContext &portableContext)
                        : context(portableContext) {
                }

                void
                PortableSerializer::read(ObjectDataInput &in, Portable &portable, int32_t factoryId, int32_t classId) {
                    int version = in.readInt();

                    int portableVersion = findPortableVersion(factoryId, classId, portable);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    portable.readPortable(reader);
                    reader.end();
                }

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) const {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == NULL) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(context, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(context, input, cd, false);
                        return reader;
                    }
                }

                int
                PortableSerializer::findPortableVersion(int factoryId, int classId, const Portable &portable) const {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion(&portable, context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                std::unique_ptr<Portable>
                PortableSerializer::createNewPortableInstance(int32_t factoryId, int32_t classId) {
                    const std::map<int32_t, std::shared_ptr<PortableFactory> > &portableFactories =
                            context.getSerializationConfig().getPortableFactories();
                    std::map<int, std::shared_ptr<hazelcast::client::serialization::PortableFactory> >::const_iterator factoryIt =
                            portableFactories.find(factoryId);

                    if (portableFactories.end() == factoryIt) {
                        return std::unique_ptr<Portable>();
                    }

                    return factoryIt->second->create(classId);
                }

                int32_t PortableSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
                }

                void PortableSerializer::write(ObjectDataOutput &out, const void *object) {
                    const Portable *p = static_cast<const Portable *>(object);

                    if (p->getClassId() == 0) {
                        throw exception::IllegalArgumentException("Portable class ID cannot be zero!");
                    }

                    out.writeInt(p->getFactoryId());
                    out.writeInt(p->getClassId());

                    writeInternal(out, p);
                }

                void PortableSerializer::writeInternal(ObjectDataOutput &out, const Portable *p) const {
                    std::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition(*p);
                    out.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(context, cd, out);
                    PortableWriter portableWriter(&dpw);
                    p->writePortable(portableWriter);
                    portableWriter.end();
                }

                void *PortableSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t PortableSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }

                SerializerHolder::SerializerHolder(const std::shared_ptr<StreamSerializer> &globalSerializer)
                        : active(true), globalSerializer(globalSerializer) {
                }

                bool SerializerHolder::registerSerializer(const std::shared_ptr<StreamSerializer> &serializer) {
                    std::shared_ptr<SerializerBase> available = serializers.putIfAbsent(
                            serializer->getHazelcastTypeId(), serializer);
                    return available.get() == NULL;
                }

                std::shared_ptr<StreamSerializer> SerializerHolder::serializerFor(int typeId) {
                    std::shared_ptr<StreamSerializer> serializer = serializers.get(typeId);

                    if (serializer.get()) {
                        return serializer;
                    }

                    serializer = lookupGlobalSerializer(typeId);

                    if (!serializer.get()) {
                        if (active) {
                            std::ostringstream out;
                            out << "There is no suitable serializer for " << typeId;
                            throw exception::HazelcastSerializationException("SerializerHolder::registerSerializer",
                                                                             out.str());
                        }
                        throw exception::HazelcastClientNotActiveException("SerializerHolder::registerSerializer");
                    }
                    return serializer;
                }

                void SerializerHolder::dispose() {
                    active.store(false);

                    for (std::shared_ptr<StreamSerializer> serializer : serializers.values()) {
                        serializer->destroy();
                    }

                    serializers.clear();
                }

                std::shared_ptr<StreamSerializer> SerializerHolder::lookupGlobalSerializer(int typeId) {
                    if (!globalSerializer.get()) {
                        return std::shared_ptr<StreamSerializer>();
                    }

                    serializers.putIfAbsent(typeId, globalSerializer);
                    return globalSerializer;
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


