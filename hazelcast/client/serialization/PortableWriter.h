//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include "BufferedDataOutput.h"
#include "FieldDefinition.h"
#include "HazelcastException.h"
#include "FieldType.h"
#include "ClassDefinitionWriter.h"
#include "SerializationContext.h"
#include <iostream>
#include <string>
#include <set>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class PortableWriter {
                template<typename T>
                friend void operator <<(PortableWriter& portableWriter, T data);

            public:

                PortableWriter(SerializationContext *serializationContext, boost::shared_ptr<ClassDefinition> cd, BufferedDataOutput *output);

                PortableWriter& operator [](std::string fieldName);

                void writeInt(int value);

                void writeLong(long value);

                void writeBoolean(bool value);

                void writeByte(byte value);

                void writeChar(int value);

                void writeDouble(double value);

                void writeFloat(float value);

                void writeShort(short value);

                void writeUTF(string str);

                void writeByteArray(std::vector<byte>&);

                void writeCharArray(std::vector<char>&);

                void writeIntArray(std::vector<int>&);

                void writeLongArray(std::vector<long>&);

                void writeDoubleArray(std::vector<double>&);

                void writeFloatArray(std::vector<float>&);

                void writeShortArray(std::vector<short>&);

                void writeNullPortable(int factoryId, int classId);

                template <typename T>
                void writePortable(T& portable) {
                    output->writeBoolean(false);
                    write(*output, portable);
                };

                template <typename T>
                void writePortableArray(std::vector<T>& values) {
                    int len = values.size();
                    output->writeInt(len);
                    if (len > 0) {
                        int offset = output->position();
                        output->position(offset + len * sizeof (int));
                        for (int i = 0; i < len; i++) {
                            output->writeInt(offset + i * sizeof (int), output->position());
                            write(*output, values[i]);
                        }
                    }
                };

            private:
                void writingToDataOutput();

                void setPosition(string fieldName);

                template <typename T>   //TODO duplicate code because of cyclic dependency look : PortableSerializer
                boost::shared_ptr<ClassDefinition> getClassDefinition(T& p) {
                    boost::shared_ptr<ClassDefinition> cd;

                    int factoryId = getFactoryId(p);
                    int classId = getClassId(p);
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        classDefinitionWriter << p;//TODO 1
                        cd = classDefinitionWriter.getClassDefinition();
                        context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, T& p) {
                    boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, &dataOutput);
                    portableWriter << p; //TODO 1
                };

                int index;
                bool raw;
                bool writingPortable;

                SerializationContext *context;
                BufferedDataOutput *output;
                int offset;
                std::set<std::string> writtenFields;
                boost::shared_ptr<ClassDefinition> cd;

            };

            template<typename T>
            inline void operator <<(PortableWriter& portableWriter, T data) {
                //TODO i probably need to add more here
                //........
                portableWriter.writingToDataOutput();
                writePortable(portableWriter, data);
            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_WRITER */
