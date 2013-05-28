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
#include "../HazelcastException.h"
#include "FieldType.h"
#include "SerializationContext.h"
#include "ClassDefinitionWriter.h"
#include "ConstantSerializers.h"
#include <string>
#include <set>
#include <vector>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class PortableWriter {
            public:

                PortableWriter(SerializationContext *serializationContext, ClassDefinition *cd, BufferedDataOutput *output);

                PortableWriter& operator [](std::string fieldName);

                void write(const std::vector<byte>& x);

                void writeInt(int value);

                void writeLong(long value);

                void writeBoolean(bool value);

                void writeByte(byte value);

                void writeChar(int value);

                void writeDouble(double value);

                void writeFloat(float value);

                void writeShort(short value);

                void writeUTF(string str);

                void writeNullPortable(int factoryId, int classId);

                template <typename T>
                void writePortable(const T& portable) {
                    output->writeBoolean(false);
                    write(*output, portable);
                };

                template <typename T>
                void writePortable(const std::vector<T>& values) {
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

                void writingToDataOutput();

            private:

                void setPosition(string fieldName);

                template <typename T>
                ClassDefinition *getClassDefinition(const T& p) {
                    ClassDefinition *cd;

                    int factoryId = getFactoryId(p);
                    int classId = getClassId(p);
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        hazelcast::client::serialization::writePortable(classDefinitionWriter, p);
                        cd = classDefinitionWriter.getClassDefinition();
                        context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(BufferedDataOutput &dataOutput, const T& p) {
                    ClassDefinition *cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, &dataOutput);
                    hazelcast::client::serialization::writePortable(portableWriter, p);
                };

                int index;
                bool raw;
                bool writingPortable;

                SerializationContext *context;
                BufferedDataOutput *output;
                int offset;
                std::set<std::string> writtenFields;
                ClassDefinition *cd;

            };

            template<typename T>
            inline void operator <<(PortableWriter& portableWriter, const std::vector<T>& data) {
                portableWriter.writingToDataOutput();
                portableWriter.writePortable(data);
            };

            template<typename T>
            inline void operator <<(PortableWriter& portableWriter, const T& data) {
                portableWriter.writingToDataOutput();
                if (getTypeId(data) == SerializationConstants::CONSTANT_TYPE_PORTABLE)
                    portableWriter.writePortable(data);
                else {
                    writePortable(portableWriter, data);
                }
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_WRITER */
