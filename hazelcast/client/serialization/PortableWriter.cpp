//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"
#include "DataOutput.h"
#include "ClassDefinition.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter() {

            };

            PortableWriter::PortableWriter(PortableSerializer* serializer, ClassDefinition* cd, DataOutput* output, Type type)
            : type(type)
            , serializer(serializer)
            , output(output)
            , index (0)
            , cd(cd) {
                if (type == DEFAULT) {
                    offset = output->position();
                    char zeros[ cd->getFieldCount() * sizeof (int) ];
                    this->output->write(zeros, 0, cd->getFieldCount() * sizeof (int));
                }

            };

            void PortableWriter::writeInt(string fieldName, int value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdInt(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeInt(value);
            };

            void PortableWriter::writeLong(string fieldName, long value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdLong(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeLong(value);
            };

            void PortableWriter::writeBoolean(string fieldName, bool value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdBoolean(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeBoolean(value);
            };

            void PortableWriter::writeByte(string fieldName, byte value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdByte(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeByte(value);
            };

            void PortableWriter::writeChar(string fieldName, int value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdChar(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeChar(value);
            };

            void PortableWriter::writeDouble(string fieldName, double value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdDouble(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeDouble(value);
            };

            void PortableWriter::writeFloat(string fieldName, float value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdFloat(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeFloat(value);
            };

            void PortableWriter::writeShort(string fieldName, short value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdShort(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeShort(value);
            };

            void PortableWriter::writeUTF(string fieldName, string value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdUTF(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                output->writeUTF(value);
            };

            void PortableWriter::writePortable(string fieldName, Portable& value) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdPortable(fieldName, value);
                    return;
                }
                setPosition(fieldName);
                bool isNull = false;
                output->writeBoolean(isNull);
                if (!isNull) {
                    serializer->write(output, value);
                }
            };

            void PortableWriter::writeByteArray(string fieldName, std::vector<byte>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdByteArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeByte(values[i]);
                    }
                }
            };

            void PortableWriter::writeCharArray(string fieldName, std::vector<char>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdCharArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeChar(values[i]);
                    }
                }
            };

            void PortableWriter::writeIntArray(string fieldName, std::vector<int>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdIntArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeInt(values[i]);
                    }
                }
            };

            void PortableWriter::writeLongArray(string fieldName, std::vector<long>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdLongArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeLong(values[i]);
                    }
                }
            };

            void PortableWriter::writeDoubleArray(string fieldName, std::vector<double>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdDoubleArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeDouble(values[i]);
                    }
                }
            };

            void PortableWriter::writeFloatArray(string fieldName, std::vector<float>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdFloatArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeFloat(values[i]);
                    }
                }
            };

            void PortableWriter::writeShortArray(string fieldName, std::vector<short>& values) {
                if (type == CLASS_DEFINITION_WRITER) {
                    getCdShortArray(fieldName, values);
                    return;
                }
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeShort(values[i]);
                    }
                }
            };

            void PortableWriter::setPosition(string fieldName) {
                if (!cd->isFieldDefinitionExists(fieldName)) {
                    std::string error;
                    error += "HazelcastSerializationException( Invalid field name: '";
                    error += fieldName;
                    error += "' for ClassDefinition {id: ";
                    error += cd->getClassId();
                    error += ", version: ";
                    error += cd->getVersion();
                    error += "}";

                    throw error;
                }
                FieldDefinition fd = cd->get(fieldName);

                int pos = output->position();
                int index = fd.getIndex();
                // index = fieldIndex++; // if class versions are the same.
                output->writeInt(offset + index * sizeof (int), pos);

            };

            void PortableWriter::getCdInt(string fieldName, int value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_INT);
                cd->add(fd);
            };

            void PortableWriter::getCdLong(string fieldName, long value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_LONG);
                cd->add(fd);
            };

            void PortableWriter::getCdUTF(string fieldName, string value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_UTF);
                cd->add(fd);
            };

            void PortableWriter::getCdBoolean(string fieldName, bool value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_BOOLEAN);
                cd->add(fd);
            };

            void PortableWriter::getCdByte(string fieldName, byte value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_BYTE);
                cd->add(fd);
            };

            void PortableWriter::getCdChar(string fieldName, int value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_CHAR);
                cd->add(fd);
            };

            void PortableWriter::getCdDouble(string fieldName, double value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_DOUBLE);
                cd->add(fd);
            };

            void PortableWriter::getCdFloat(string fieldName, float value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_FLOAT);
                cd->add(fd);
            };

            void PortableWriter::getCdShort(string fieldName, short value) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_SHORT);
                cd->add(fd);
            };

            void PortableWriter::getCdPortable(string fieldName, Portable& portable) {
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldDefinition::TYPE_PORTABLE, portable.getClassId());
                addNestedField(portable, fd);
            };

            void PortableWriter::getCdByteArray(string fieldName, std::vector<byte>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_BYTE_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdCharArray(string fieldName, std::vector<char>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_CHAR_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdIntArray(string fieldName, std::vector<int>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_INT_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdLongArray(string fieldName, std::vector<long>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_LONG_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdDoubleArray(string fieldName, std::vector<double>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_DOUBLE_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdFloatArray(string fieldName, std::vector<float>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_FLOAT_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::getCdShortArray(string fieldName, std::vector<short>& v) {
                FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_SHORT_ARRAY);
                cd->add(fd);
            };

            void PortableWriter::addNestedField(Portable& p, FieldDefinition& fd) {
                cd->add(fd);
                ClassDefinition* nestedCd = serializer->getClassDefinition(p);

                cd->add(nestedCd);
            };

        }
    }
}