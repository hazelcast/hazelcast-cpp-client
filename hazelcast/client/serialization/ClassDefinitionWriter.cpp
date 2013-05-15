//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include <boost/smart_ptr/shared_ptr.hpp>
#include "ClassDefinitionWriter.h"
#include "PortableSerializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ClassDefinitionWriter::ClassDefinitionWriter(int factoryId, int classId, int version, SerializationContext *serializationContext)
            : factoryId(factoryId)
            , classId(classId)
            , raw(false)
            , writingPortable(false)
            , context(serializationContext)
            , index(0)
            , cd(new ClassDefinition(factoryId, classId, version)) {
            };


            boost::shared_ptr<ClassDefinition>  ClassDefinitionWriter::getClassDefinition() {
                return cd;
            };

            ClassDefinitionWriter& ClassDefinitionWriter::operator [](std::string fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot call [] operation after writing directly to stream(without [])");
                }
                lastFieldName = fieldName;
                writingPortable = true;
                return *this;
            };

            //TODO need more thought on above and below functions
            void ClassDefinitionWriter::addField(FieldType const & fieldType) {
                if (writingPortable) {
                    FieldDefinition fd(index++, lastFieldName, fieldType);
                    cd->add(fd);
                    writingPortable = false;
                } else {
                    raw = true;
                }
            }

            void ClassDefinitionWriter::writeInt(int value) {
                addField(FieldTypes::TYPE_INT);
            };

            void ClassDefinitionWriter::writeLong(long value) {
                addField(FieldTypes::TYPE_LONG);

            };

            void ClassDefinitionWriter::writeBoolean(bool value) {
                addField(FieldTypes::TYPE_BOOLEAN);

            };

            void ClassDefinitionWriter::writeByte(byte value) {
                addField(FieldTypes::TYPE_BYTE);

            };

            void ClassDefinitionWriter::writeChar(int value) {
                addField(FieldTypes::TYPE_CHAR);

            };

            void ClassDefinitionWriter::writeDouble(double value) {
                addField(FieldTypes::TYPE_DOUBLE);

            };

            void ClassDefinitionWriter::writeFloat(float value) {
                addField(FieldTypes::TYPE_FLOAT);

            };

            void ClassDefinitionWriter::writeShort(short value) {
                addField(FieldTypes::TYPE_SHORT);

            };

            void ClassDefinitionWriter::writeUTF(string value) {
                addField(FieldTypes::TYPE_UTF);

            };

            void ClassDefinitionWriter::writeNullPortable(int factoryId, int classId) {
                if (!raw) {
                    FieldDefinition fd = FieldDefinition(index++, lastFieldName, FieldTypes::TYPE_PORTABLE, factoryId, classId);
                    if (context->isClassDefinitionExists(factoryId, classId) == false) {
                        throw hazelcast::client::HazelcastException("Cannot write null portable withouy explicitly registering class definition!");
                    } else {
                        cd->add(fd);
                        cd->add(context->lookup(factoryId, classId));
                    }
                }

            };

            void ClassDefinitionWriter::writeByteArray(std::vector<byte>& values) {
                addField(FieldTypes::TYPE_BYTE_ARRAY);

            };

            void ClassDefinitionWriter::writeCharArray(std::vector<char>& values) {
                addField(FieldTypes::TYPE_CHAR_ARRAY);

            };

            void ClassDefinitionWriter::writeIntArray(std::vector<int>& values) {
                addField(FieldTypes::TYPE_INT_ARRAY);

            };

            void ClassDefinitionWriter::writeLongArray(std::vector<long>& values) {
                addField(FieldTypes::TYPE_LONG_ARRAY);

            };

            void ClassDefinitionWriter::writeDoubleArray(std::vector<double>& values) {
                addField(FieldTypes::TYPE_DOUBLE_ARRAY);

            };

            void ClassDefinitionWriter::writeFloatArray(std::vector<float>& values) {
                addField(FieldTypes::TYPE_FLOAT_ARRAY);

            };

            void ClassDefinitionWriter::writeShortArray(std::vector<short>& values) {
                addField(FieldTypes::TYPE_SHORT_ARRAY);

            };

        }
    }
}