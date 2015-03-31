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
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

using namespace hazelcast::client::serialization;
using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableWriter::DefaultPortableWriter(PortableContext& portableContext, boost::shared_ptr<ClassDefinition> cd, DataOutput& dataOutput)
                : raw(false)
                , serializerHolder(portableContext.getSerializerHolder())
                , dataOutput(dataOutput)
                , objectDataOutput(dataOutput, portableContext)
                , begin(dataOutput.position())
                , cd(cd) {
                    // room for final offset
                    objectDataOutput.writeZeroBytes(4);

                    objectDataOutput.writeInt(cd->getFieldCount());

                    offset = dataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * Bits::INT_SIZE_IN_BYTES;
                    objectDataOutput.writeZeroBytes(fieldIndexesLength);
                }

                void DefaultPortableWriter::writeInt(const char *fieldName, int value) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    dataOutput.writeInt(value);
                }

                void DefaultPortableWriter::writeLong(const char *fieldName, long value) {
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

                void DefaultPortableWriter::writeChar(const char *fieldName, int value) {
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

                void DefaultPortableWriter::writeShort(const char *fieldName, short value) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    dataOutput.writeShort(value);
                }

                void DefaultPortableWriter::writeUTF(const char *fieldName, const std::string& value) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    dataOutput.writeUTF(value);
                }

                void DefaultPortableWriter::writeByteArray(const char *fieldName, const std::vector<byte>& bytes) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    dataOutput.writeByteArray(bytes);
                }

                void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char>& data) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    dataOutput.writeCharArray(data);
                }

                void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<short>& data) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    dataOutput.writeShortArray(data);
                }

                void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int>& data) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    dataOutput.writeIntArray(data);
                }

                void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<long>& data) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    dataOutput.writeLongArray(data);
                }

                void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float>& data) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    dataOutput.writeFloatArray(data);
                }

                void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double>& data) {
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
                        int pos = dataOutput.position();
                        int index = fd.getIndex();
                        dataOutput.writeInt(offset + index * Bits::INT_SIZE_IN_BYTES, pos);
                        size_t nameLen = strlen(fieldName);
                        dataOutput.writeShort((short)nameLen);
                        dataOutput.writeBytes((byte *)fieldName, nameLen);
                        dataOutput.writeByte(fieldType.getId());

                        return fd;

                    } catch (hazelcast::client::exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << hazelcast::util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + hazelcast::util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << hazelcast::util::IOUtil::to_string(cd->getVersion()) << "}";

                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", error.str());
                    }

                }


                ObjectDataOutput& DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        int pos = dataOutput.position();
                        int index = cd->getFieldCount(); // last index
                        dataOutput.writeInt(offset + index * Bits::INT_SIZE_IN_BYTES, pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt(begin, dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable& p) {
                    serializerHolder.getPortableSerializer().writeInternal(dataOutput, p);
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
