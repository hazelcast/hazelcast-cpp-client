//
//  ClassDefinitionWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/serialization/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ClassDefinitionWriter::ClassDefinitionWriter(int factoryId, int classId, int version, SerializationContext& serializationContext)
            : factoryId(factoryId)
            , classId(classId)
            , raw(false)
            , context(serializationContext)
            , index(0)
            , cd(new ClassDefinition(factoryId, classId, version)) {
            };


            ClassDefinition *ClassDefinitionWriter::getClassDefinition() {
                return cd;
            };

            void ClassDefinitionWriter::addField(const char *fieldName, FieldType const & fieldType) {
                if (raw) {
                    throw exception::IOException("ClassDefinitionWriter::addField(", "Cannot write Portable fields after getRawDataOutput() is called!");
                }
                FieldDefinition fd(index++, fieldName, fieldType);
                cd->add(fd);
            };

            void ClassDefinitionWriter::writeInt(const char *fieldName, int value) {
                addField(fieldName, FieldTypes::TYPE_INT);
            };

            void ClassDefinitionWriter::writeLong(const char *fieldName, long value) {
                addField(fieldName, FieldTypes::TYPE_LONG);

            };

            void ClassDefinitionWriter::writeBoolean(const char *fieldName, bool value) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN);

            };

            void ClassDefinitionWriter::writeByte(const char *fieldName, byte value) {
                addField(fieldName, FieldTypes::TYPE_BYTE);

            };

            void ClassDefinitionWriter::writeChar(const char *fieldName, int value) {
                addField(fieldName, FieldTypes::TYPE_CHAR);

            };

            void ClassDefinitionWriter::writeDouble(const char *fieldName, double value) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE);

            };

            void ClassDefinitionWriter::writeFloat(const char *fieldName, float value) {
                addField(fieldName, FieldTypes::TYPE_FLOAT);

            };

            void ClassDefinitionWriter::writeShort(const char *fieldName, short value) {
                addField(fieldName, FieldTypes::TYPE_SHORT);

            };

            void ClassDefinitionWriter::writeUTF(const char *fieldName, const std::string& value) {
                addField(fieldName, FieldTypes::TYPE_UTF);

            };

            void ClassDefinitionWriter::writeNullPortable(const char *fieldName, int factoryId, int classId) {
                if (!raw) {
                    FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, factoryId, classId);
                    if (context.isClassDefinitionExists(factoryId, classId) == false) {
                        throw exception::IOException("ClassDefinitionWriter::writeNullPortable", "Cannot write null portable withouy explicitly registering class definition!");
                    } else {
                        cd->add(fd);
                        cd->add(context.lookup(factoryId, classId));
                    }
                }

            };

            void ClassDefinitionWriter::writeByteArray(const char *fieldName, const std::vector<byte>& values) {
                addField(fieldName, FieldTypes::TYPE_BYTE_ARRAY);

            };

            void ClassDefinitionWriter::writeCharArray(const char *fieldName, const std::vector<char>& values) {
                addField(fieldName, FieldTypes::TYPE_CHAR_ARRAY);

            };

            void ClassDefinitionWriter::writeIntArray(const char *fieldName, const std::vector<int>& values) {
                addField(fieldName, FieldTypes::TYPE_INT_ARRAY);

            };

            void ClassDefinitionWriter::writeLongArray(const char *fieldName, const std::vector<long>& values) {
                addField(fieldName, FieldTypes::TYPE_LONG_ARRAY);

            };

            void ClassDefinitionWriter::writeDoubleArray(const char *fieldName, const std::vector<double>& values) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);

            };

            void ClassDefinitionWriter::writeFloatArray(const char *fieldName, const std::vector<float>& values) {
                addField(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);

            };

            void ClassDefinitionWriter::writeShortArray(const char *fieldName, const std::vector<short>& values) {
                addField(fieldName, FieldTypes::TYPE_SHORT_ARRAY);

            };

            ObjectDataOutput& ClassDefinitionWriter::getRawDataOutput() {
                return emptyDataOutput;
            };

            void ClassDefinitionWriter::end() {

            };

            ClassDefinition *ClassDefinitionWriter::getOrBuildClassDefinition(const Portable& p) {
                return context.getSerializerHolder().getPortableSerializer().getClassDefinition(p);
            };


        }

    }
}