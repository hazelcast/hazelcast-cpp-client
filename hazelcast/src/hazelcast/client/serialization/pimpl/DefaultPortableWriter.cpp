//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include <hazelcast/client/exception/HazelcastSerializationException.h>
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"

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
                , offset(dataOutput.position() + sizeof(int))
                , cd(cd) {
                    int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof(int);
                    this->dataOutput.position(offset + fieldIndexesLength);
                }

                void DefaultPortableWriter::writeInt(const char *fieldName, int value) {
                    setPosition(fieldName);
                    dataOutput.writeInt(value);
                }

                void DefaultPortableWriter::writeLong(const char *fieldName, long value) {
                    setPosition(fieldName);
                    dataOutput.writeLong(value);
                }

                void DefaultPortableWriter::writeBoolean(const char *fieldName, bool value) {
                    setPosition(fieldName);
                    dataOutput.writeBoolean(value);
                }

                void DefaultPortableWriter::writeByte(const char *fieldName, byte value) {
                    setPosition(fieldName);
                    dataOutput.writeByte(value);
                }

                void DefaultPortableWriter::writeChar(const char *fieldName, int value) {
                    setPosition(fieldName);
                    dataOutput.writeChar(value);
                }

                void DefaultPortableWriter::writeDouble(const char *fieldName, double value) {
                    setPosition(fieldName);
                    dataOutput.writeDouble(value);
                }

                void DefaultPortableWriter::writeFloat(const char *fieldName, float value) {
                    setPosition(fieldName);
                    dataOutput.writeFloat(value);
                }

                void DefaultPortableWriter::writeShort(const char *fieldName, short value) {
                    setPosition(fieldName);
                    dataOutput.writeShort(value);
                }

                void DefaultPortableWriter::writeUTF(const char *fieldName, const std::string& value) {
                    setPosition(fieldName);
                    dataOutput.writeUTF(value);
                }

                void DefaultPortableWriter::writeByteArray(const char *fieldName, const std::vector<byte>& bytes) {
                    setPosition(fieldName);
                    dataOutput.writeByteArray(bytes);
                }

                void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char>& data) {
                    setPosition(fieldName);
                    dataOutput.writeCharArray(data);
                }

                void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<short>& data) {
                    setPosition(fieldName);
                    dataOutput.writeShortArray(data);
                }

                void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int>& data) {
                    setPosition(fieldName);
                    dataOutput.writeIntArray(data);
                }

                void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<long>& data) {
                    setPosition(fieldName);
                    dataOutput.writeLongArray(data);
                }

                void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float>& data) {
                    setPosition(fieldName);
                    dataOutput.writeFloatArray(data);
                }

                void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double>& data) {
                    setPosition(fieldName);
                    dataOutput.writeDoubleArray(data);
                }

                FieldDefinition const& DefaultPortableWriter::setPosition(const char *fieldName) {
                    if (raw) throw exception::HazelcastSerializationException("PortableWriter::setPosition", "Cannot write Portable fields after getRawDataOutput() is called!");
                    if (!cd->hasField(fieldName)) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}";

                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", error.str());
                    }

                    if (writtenFields.count(fieldName) != 0) {
                        throw exception::HazelcastSerializationException("PortableWriter::setPosition", "Field '" + std::string(fieldName) + "' has already been written!");
                    }
                    writtenFields.insert(fieldName);
                    FieldDefinition const& field = cd->getField(fieldName);
                    dataOutput.writeInt(offset + field.getIndex() * sizeof(int), dataOutput.position());
                    return field;

                }


                ObjectDataOutput& DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        int pos = dataOutput.position();
                        int index = cd->getFieldCount(); // last index
                        dataOutput.writeInt(offset + index * sizeof(int), pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    dataOutput.writeInt(begin, dataOutput.position()); // write final offset
                }

                void DefaultPortableWriter::write(const Portable& p) {
                    return serializerHolder.getPortableSerializer().write(dataOutput, p);
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
