//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER


#include "PortableWriter.h"
#include <iostream>
#include <string>

using namespace std;
namespace hazelcast {
    namespace client {
        namespace serialization {

            class FieldDefinition;
            class Portable;
            class PortableSerializer;
            class ClassDefinition;

            class ClassDefinitionWriter : public PortableWriter {
            public:
                ClassDefinitionWriter(PortableSerializer* serializer, ClassDefinition* cd);

                void writeInt(string fieldName, int value);
                void writeLong(string fieldName, long value);
                void writeUTF(string fieldName, string str);
                void writeBoolean(string fieldName, bool value);
                void writeByte(string fieldName, byte value);
                void writeChar(string fieldName, int value);
                void writeDouble(string fieldName, double value);
                void writeFloat(string fieldName, float value);
                void writeShort(string fieldName, short value);
                void writePortable(string fieldName, Portable& portable);
                void writeByteArray(string fieldName, std::vector<byte>&);
                void writeCharArray(string fieldName, std::vector<char>&);
                void writeIntArray(string fieldName, std::vector<int>&);
                void writeLongArray(string fieldName, std::vector<long>&);
                void writeDoubleArray(string fieldName, std::vector<double>&);
                void writeFloatArray(string fieldName, std::vector<float>&);
                void writeShortArray(string fieldName, std::vector<short>&);
                void writePortableArray(string fieldName, std::vector<Portable*>& portables);

                ClassDefinition* cd;
            private:
                void addNestedField(Portable& p, FieldDefinition& fd);

                int index;
                PortableSerializer* serializer;

            };

        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
