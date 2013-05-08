//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"
#include "ClassDefinition.h"
#include "EmptyDataOutput.h"
#include "StringUtil.h"
#include "SerializationContext.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter(PortableSerializer *serializer, boost::shared_ptr<ClassDefinition> cd, DataOutput *output, Type type)
            : type(type)
            , serializer(serializer)
            , output(output)
            , index (0)
            , cd(cd)
            , raw(false) {
                if (type == DEFAULT) {
                    offset = output->position();
                    int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                    this->output->position(offset + fieldIndexesLength);
                }

            };

            void PortableWriter::writeInt(string fieldName, int value) {
                if (checkType(fieldName, FieldTypes::TYPE_INT)) return;
                setPosition(fieldName);
                output->writeInt(value);
            };

            void PortableWriter::writeLong(string fieldName, long value) {
                if (checkType(fieldName, FieldTypes::TYPE_LONG)) return;
                setPosition(fieldName);
                output->writeLong(value);
            };

            void PortableWriter::writeBoolean(string fieldName, bool value) {
                if (checkType(fieldName, FieldTypes::TYPE_BOOLEAN)) return;
                setPosition(fieldName);
                output->writeBoolean(value);
            };

            void PortableWriter::writeByte(string fieldName, byte value) {
                if (checkType(fieldName, FieldTypes::TYPE_BYTE)) return;
                setPosition(fieldName);
                output->writeByte(value);
            };

            void PortableWriter::writeChar(string fieldName, int value) {
                if (checkType(fieldName, FieldTypes::TYPE_CHAR)) return;
                setPosition(fieldName);
                output->writeChar(value);
            };

            void PortableWriter::writeDouble(string fieldName, double value) {
                if (checkType(fieldName, FieldTypes::TYPE_DOUBLE)) return;
                setPosition(fieldName);
                output->writeDouble(value);
            };

            void PortableWriter::writeFloat(string fieldName, float value) {
                if (checkType(fieldName, FieldTypes::TYPE_FLOAT)) return;
                setPosition(fieldName);
                output->writeFloat(value);
            };

            void PortableWriter::writeShort(string fieldName, short value) {
                if (checkType(fieldName, FieldTypes::TYPE_SHORT)) return;
                setPosition(fieldName);
                output->writeShort(value);
            };

            void PortableWriter::writeUTF(string fieldName, string value) {
                if (checkType(fieldName, FieldTypes::TYPE_UTF)) return;
                setPosition(fieldName);
                output->writeUTF(value);
            };

            void PortableWriter::writePortable(string fieldName, Portable& value) {
                if (checkType(fieldName, value)) return;
                setPosition(fieldName);
                output->writeBoolean(false);
                serializer->write(*output, value);
            };


            void PortableWriter::writeNullPortable(string fieldName, int factoryId, int classId) {
                if (checkType(fieldName, factoryId, classId)) return;
                setPosition(fieldName);
                output->writeBoolean(true);
            };

            void PortableWriter::writeByteArray(string fieldName, std::vector<byte>& values) {
                if (checkType(fieldName, FieldTypes::TYPE_BYTE_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_CHAR_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_INT_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_LONG_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_FLOAT_ARRAY)) return;
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
                if (checkType(fieldName, FieldTypes::TYPE_SHORT_ARRAY)) return;
                setPosition(fieldName);
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeShort(values[i]);
                    }
                }
            };

            DataOutput *const hazelcast::client::serialization::PortableWriter::getRawDataOutput() {
                if (type == CLASS_DEFINITION_WRITER) {
                    static EmptyDataOutput emptyDataOutput;
                    return &emptyDataOutput;
                }
                if (!raw) {
                    int pos = output->position();
                    int index = cd->getFieldCount(); // last index
                    output->writeInt(offset + index * sizeof(int), pos);
                }
                raw = true;
                return output;
            }

            void PortableWriter::setPosition(string fieldName) {
                if (raw) throw HazelcastException("Cannot write Portable fields after getRawDataOutput() is called!");
                if (!cd->isFieldDefinitionExists(fieldName)) {
                    std::string error;
                    error += "HazelcastSerializationException( Invalid field name: '";
                    error += fieldName;
                    error += "' for ClassDefinition {id: ";
                    error += hazelcast::client::util::StringUtil::to_string(cd->getClassId());
                    error += ", version: ";
                    error += hazelcast::client::util::StringUtil::to_string(cd->getVersion());
                    error += "}";

                    throw hazelcast::client::HazelcastException(error);
                }
                FieldDefinition fd = cd->get(fieldName);

                if (writtenFields.count(fieldName) != 0)
                    hazelcast::client::HazelcastException("Field '" + fieldName + "' has already been written!");
                writtenFields.insert(fieldName);
                int pos = output->position();
                int index = fd.getIndex();
                output->writeInt(offset + index * sizeof (int), pos);

            };

            bool PortableWriter::checkType(string fieldName, FieldType const& fieldType) {
                if (type == CLASS_DEFINITION_WRITER) {
                    if (!raw) {
                        FieldDefinition fd(index++, fieldName, fieldType);
                        cd->add(fd);
                    }
                    return true;
                } else {
                    return false;
                }
            }

            bool PortableWriter::checkType(string fieldName, int factoryId, int classId) {
                if (type == CLASS_DEFINITION_WRITER) {
                    if (!raw) {
                        FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, factoryId, classId);
                        SerializationContext *pContext = serializer->getSerializationContext();
                        if (pContext->isClassDefinitionExists(factoryId, classId) == false) {
                            throw hazelcast::client::HazelcastException("Cannot write null portable withouy explicitly registering class definition!");
                        } else {
                            cd->add(fd);
                            cd->add(pContext->lookup(factoryId, classId));
                        }
                    }
                    return true;
                } else {
                    return false;
                }
            }

            bool PortableWriter::checkType(string fieldName, Portable& portable) {
                if (type == CLASS_DEFINITION_WRITER) {
                    if (!raw) {
                        FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, portable.getFactoryId(), portable.getClassId());
                        addNestedField(portable, fd);
                    }
                    return true;
                } else {
                    return false;
                }
            }

            void PortableWriter::addNestedField(Portable& p, FieldDefinition& fd) {
                cd->add(fd);
                boost::shared_ptr<ClassDefinition> nestedCd = serializer->getClassDefinition(p);
                cd->add(nestedCd);
            };

        }
    }
}