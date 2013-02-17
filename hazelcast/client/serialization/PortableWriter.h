//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include "DataOutput.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"
#include <iostream>
#include <string>
using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;
            class DataInput;
            class Portable;

            class PortableWriter {
            public:

                enum Type {
                    DEFAULT, CLASS_DEFINITION_WRITER
                };

                PortableWriter();
                PortableWriter(PortableSerializer* serializer, ClassDefinition* cd, DataOutput* output, Type type);

                void writeInt(string fieldName, int value);
                void writeLong(string fieldName, long value);
                void writeBoolean(string fieldName, bool value);
                void writeByte(string fieldName, byte value);
                void writeChar(string fieldName, int value);
                void writeDouble(string fieldName, double value);
                void writeFloat(string fieldName, float value);
                void writeShort(string fieldName, short value);
                void writeUTF(string fieldName, string str);
                void writePortable(string fieldName, Portable& portable);
                void writeByteArray(string fieldName, std::vector<byte>&);
                void writeCharArray(string fieldName, std::vector<char>&);
                void writeIntArray(string fieldName, std::vector<int>&);
                void writeLongArray(string fieldName, std::vector<long>&);
                void writeDoubleArray(string fieldName, std::vector<double>&);
                void writeFloatArray(string fieldName, std::vector<float>&);
                void writeShortArray(string fieldName, std::vector<short>&);

                template <typename T>
                void writePortableArray(string fieldName, std::vector<T>& values) {
                    if (type == CLASS_DEFINITION_WRITER) {
                        getCdPortableArray(fieldName, values);
                        return;
                    }
                    setPosition(fieldName);
                    int len = values.size();
                    output->writeInt(len);
                    if (len > 0) {
                        int offset = output->position();
                        char zeros[len * sizeof (int) ];
                        output->write(zeros, 0, len * sizeof (int));
                        for (int i = 0; i < len; i++) {
                            output->writeInt(offset + i * sizeof (int), output->position());
                            serializer->write(output, values[i]);
                        }
                    }
                };

                void setPosition(string fieldName);

                void getCdInt(string fieldName, int value);
                void getCdLong(string fieldName, long value);
                void getCdUTF(string fieldName, string str);
                void getCdBoolean(string fieldName, bool value);
                void getCdByte(string fieldName, byte value);
                void getCdChar(string fieldName, int value);
                void getCdDouble(string fieldName, double value);
                void getCdFloat(string fieldName, float value);
                void getCdShort(string fieldName, short value);
                void getCdPortable(string fieldName, Portable& portable);
                void getCdByteArray(string fieldName, std::vector<byte>&);
                void getCdCharArray(string fieldName, std::vector<char>&);
                void getCdIntArray(string fieldName, std::vector<int>&);
                void getCdLongArray(string fieldName, std::vector<long>&);
                void getCdDoubleArray(string fieldName, std::vector<double>&);
                void getCdFloatArray(string fieldName, std::vector<float>&);
                void getCdShortArray(string fieldName, std::vector<short>&);
                template<typename T>
                void getCdPortableArray(string fieldName, std::vector<T>& portables) {
                    int classId = portables[0].getClassId();
                    for (int i = 1; i < portables.size(); i++) {
                        if (portables[i].getClassId() != classId) {
                            throw "Illegal Argument Exception";
                        }
                    }
                    FieldDefinition fd(index++, fieldName, FieldDefinition::TYPE_PORTABLE_ARRAY, classId);
                    addNestedField(portables[0], fd);
                };
                void addNestedField(Portable& p, FieldDefinition& fd);

            private:
                Type type;
                int index;
                PortableSerializer* serializer;
                DataOutput* output;
                int offset;
                ClassDefinition* cd;
            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_WRITER */
