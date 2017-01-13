/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by İhsan Demir on 25/03/15.
//

#include "hazelcast/util/Util.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {


                PortableReaderBase::PortableReaderBase(PortableContext& portableContext, DataInput& input, boost::shared_ptr<ClassDefinition> cd)
                : cd(cd)
                , dataInput(input)
                , serializerHolder(portableContext.getSerializerHolder())
                , objectDataInput(input, portableContext)
                , raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.readInt();
                        // field count
                        fieldCount = input.readInt();
                    } catch (exception::IException& e) {
                        throw exception::HazelcastSerializationException("[DefaultPortableReader::DefaultPortableReader]", e.what());
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount, cd->getFieldCount());
                        throw new exception::IllegalStateException("[DefaultPortableReader::DefaultPortableReader]", msg);
                    }
                    this->offset = input.position();
                }

                PortableReaderBase::~PortableReaderBase() {

                }

                int PortableReaderBase::readInt(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    return dataInput.readInt();
                }

                long PortableReaderBase::readLong(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    return (long)dataInput.readLong();
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

                short PortableReaderBase::readShort(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    return dataInput.readShort();
                }

                std::auto_ptr<std::string> PortableReaderBase::readUTF(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    return dataInput.readUTF();
                }

                std::auto_ptr<std::vector<byte> > PortableReaderBase::readByteArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    return dataInput.readByteArray();
                }

                std::auto_ptr<std::vector<char> > PortableReaderBase::readCharArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    return dataInput.readCharArray();
                }

                std::auto_ptr<std::vector<int> > PortableReaderBase::readIntArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    return dataInput.readIntArray();
                }

                std::auto_ptr<std::vector<long> > PortableReaderBase::readLongArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    return dataInput.readLongArray();
                }

                std::auto_ptr<std::vector<double> > PortableReaderBase::readDoubleArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    return dataInput.readDoubleArray();
                }

                std::auto_ptr<std::vector<float> > PortableReaderBase::readFloatArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    return dataInput.readFloatArray();
                }

                std::auto_ptr<std::vector<short> > PortableReaderBase::readShortArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    return dataInput.readShortArray();
                }

                void PortableReaderBase::getPortableInstance(char const *fieldName, Portable *& portableInstance) {
                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE);

                    bool isNull = dataInput.readBoolean();
                    int factoryId = dataInput.readInt();
                    int classId = dataInput.readInt();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (isNull) {
                        portableInstance = NULL;
                    } else {
                        read(dataInput, *portableInstance, factoryId, classId);
                    }
                }

                void PortableReaderBase::getPortableInstancesArray(char const *fieldName, std::vector<Portable *>& portableInstances) {
                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    int len = dataInput.readInt();
                    int factoryId = dataInput.readInt();
                    int classId = dataInput.readInt();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (len > 0) {
                        int offset = dataInput.position();
                        for (int i = 0; i < len; i++) {
                            dataInput.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int start = dataInput.readInt();
                            dataInput.position(start);

                            read(dataInput, *(portableInstances[i]), factoryId, classId);
                        }
                    }
                }


                void PortableReaderBase::setPosition(char const *fieldName, FieldType const& fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int PortableReaderBase::readPosition(const char *fieldName, FieldType const& fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Cannot read Portable fields after getRawDataInput() is called!");
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Don't have a field named " + std::string(fieldName));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Field type did not matched for " + std::string(fieldName));
                    }

                    dataInput.position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int pos = dataInput.readInt();

                    dataInput.position(pos);
                    short len = dataInput.readShort();

                    // name + len + type
                    return pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1;
                }

                hazelcast::client::serialization::ObjectDataInput& PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int pos = dataInput.readInt();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return objectDataInput;
                }

                void PortableReaderBase::read(DataInput& dataInput, Portable& object, int factoryId, int classId) const {
                    serializerHolder.getPortableSerializer().read(dataInput, object, factoryId, classId);
                }

                void PortableReaderBase::end() {
                    dataInput.position(finalPosition);
                }

                void PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(), factoryId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ", std::string(msg));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(), classId);
                        throw exception::HazelcastSerializationException("DefaultPortableReader::checkFactoryAndClass ", std::string(msg));
                    }
                }

            }
        }
    }
}


