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
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include <string.h>
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableWriter::DefaultPortableWriter(PortableContext& portableContext, boost::shared_ptr<ClassDefinition> cd, ObjectDataOutput &output)
                : raw(false)
                , serializerHolder(portableContext.getSerializerHolder())
                , dataOutput(*output.getDataOutput())
                , objectDataOutput(output)
                , begin(dataOutput.position())
                , cd(cd) {
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

                FieldDefinition const& DefaultPortableWriter::setPosition(const char *fieldName, FieldType fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", "Cannot write Portable fields after getRawDataOutput() is called!");
                    }

                    try {
                        FieldDefinition const& fd = cd->getField(fieldName);

                        if (writtenFields.count(fieldName) != 0) {
                            throw exception::HazelcastSerializationException("PortableWriter::setPosition", "Field '" + std::string(fieldName) + "' has already been written!");
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = dataOutput.position();
                        int32_t index = fd.getIndex();
                        dataOutput.writeInt((int32_t)(offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t)pos);
                        size_t nameLen = strlen(fieldName);
                        dataOutput.writeShort(nameLen);
                        dataOutput.writeBytes((byte *)fieldName, nameLen);
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


                ObjectDataOutput& DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = dataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        dataOutput.writeInt((int32_t)(offset + index * util::Bits::INT_SIZE_IN_BYTES), (int32_t)pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt((int32_t)begin, (int32_t)dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable& p) {
                    boost::shared_ptr<PortableSerializer> serializer = boost::static_pointer_cast<PortableSerializer>(
                            serializerHolder.serializerFor(SerializationConstants::CONSTANT_TYPE_PORTABLE));
                    serializer->write(objectDataOutput, p);
                }


                void DefaultPortableWriter::checkPortableAttributes(const FieldDefinition& fd, const Portable& portable) {
                    if (fd.getFactoryId() != portable.getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                        << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: " << portable.getFactoryId();
                        throw exception::HazelcastSerializationException("DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                    if (fd.getClassId() != portable.getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                        << "Expected class-id: " << fd.getClassId() << ", Actual class-id: " << portable.getClassId();
                        throw exception::HazelcastSerializationException("DefaultPortableWriter::::checkPortableAttributes", errorMessage.str());
                    }
                }
            }
        }
    }
}
