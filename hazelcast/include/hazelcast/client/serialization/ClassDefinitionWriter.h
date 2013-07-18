//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER

#include "FieldDefinition.h"
#include "IException.h"
#include "FieldType.h"
#include "ClassDefinition.h"
#include "SerializationContext.h"
#include "EmptyDataOutput.h"
#include "ConstantSerializers.h"
#include <string>
#include <iosfwd>


namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinitionWriter {
            public:

                ClassDefinitionWriter(int factoryId, int classId, int version, SerializationContext *serializationContext);

                void writeInt(const char *fieldName, int value);

                void writeLong(const char *fieldName, long value);

                void writeBoolean(const char *fieldName, bool value);

                void writeByte(const char *fieldName, byte value);

                void writeChar(const char *fieldName, int value);

                void writeDouble(const char *fieldName, double value);

                void writeFloat(const char *fieldName, float value);

                void writeShort(const char *fieldName, short value);

                void writeUTF(const char *fieldName, const string& str);

                void writeNullPortable(const char *fieldName, int factoryId, int classId);

                void writeByteArray(const char *fieldName, const std::vector<byte>& values);

                void writeCharArray(const char *fieldName, const std::vector<char >&  data);

                void writeShortArray(const char *fieldName, const std::vector<short >&  data);

                void writeIntArray(const char *fieldName, const std::vector<int>&  data);

                void writeLongArray(const char *fieldName, const std::vector<long >&  data);

                void writeFloatArray(const char *fieldName, const std::vector<float >&  data);

                void writeDoubleArray(const char *fieldName, const std::vector<double >&  data);

                template <typename T>
                void writePortable(const char *fieldName, const T& portable) {
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE, portable.getFactoryId(), portable.getClassId());
                    addNestedField(portable, fd);
                };

                template <typename T>
                void writePortableArray(const char *fieldName, const std::vector<T>& portables) {
                    int classId = portables[0].getClassId();
                    int factoryId = portables[0].getFactoryId();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
                    addNestedField(portables[0], fd);
                };

                ClassDefinition *getClassDefinition();

                BufferedDataOutput *getRawDataOutput();

            private:
                void addField(const char *fieldName, FieldType const&);

                template <typename T>
                void addNestedField(T& p, FieldDefinition& fd) {
                    cd->add(fd);
                    ClassDefinition *nestedCd = getOrBuildClassDefinition(p);
                    cd->add(nestedCd);
                };

                template <typename T>
                ClassDefinition *getOrBuildClassDefinition(const T& p) {
                    ClassDefinition *cd;

                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        p.writePortable(classDefinitionWriter);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                int factoryId;
                int classId;
                int index;
                bool raw;
                ClassDefinition *cd;
                SerializationContext *context;
                EmptyDataOutput emptyDataOutput;

            };

        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
