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
#include "FieldDefinition.h"
#include "IException.h"
#include "FieldType.h"
#include "SerializationContext.h"
#include "ClassDefinitionWriter.h"
#include "ConstantSerializers.h"
#include "SerializationConstraints.h"
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

                PortableWriter(SerializationContext& serializationContext, util::AtomicPointer<ClassDefinition> cd, DataOutput& output);

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

                void writeByteArray(const char *fieldName, const std::vector<byte>& x);

                void writeCharArray(const char *fieldName, const std::vector<char >&  data);

                void writeShortArray(const char *fieldName, const std::vector<short >&  data);

                void writeIntArray(const char *fieldName, const std::vector<int>&  data);

                void writeLongArray(const char *fieldName, const std::vector<long >&  data);

                void writeFloatArray(const char *fieldName, const std::vector<float >&  data);

                void writeDoubleArray(const char *fieldName, const std::vector<double >&  data);

                void end();

                template <typename T>
                void writePortable(const char *fieldName, const T& portable) {
                    Is_Portable<T>();
                    setPosition(fieldName);
                    dataOutput.writeBoolean(false);
                    write(portable);
                };

                template <typename T>
                void writePortableArray(const char *fieldName, const std::vector<T>& values) {
                    Is_Portable<T>();
                    setPosition(fieldName);
                    int len = values.size();
                    dataOutput.writeInt(len);
                    if (len > 0) {
                        int offset = dataOutput.position();
                        dataOutput.position(offset + len * sizeof (int));
                        for (int i = 0; i < len; i++) {
                            dataOutput.writeInt(offset + i * sizeof (int), dataOutput.position());
                            write(values[i]);
                        }
                    }
                };

                ObjectDataOutput *getRawDataOutput();

            private:

                void setPosition(const char *fieldName);

                template <typename T>
                util::AtomicPointer<ClassDefinition> getClassDefinition(const T& p) {
                    util::AtomicPointer<ClassDefinition> cd;

                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context.isClassDefinitionExists(factoryId, classId)) {
                        cd = context.lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context.getVersion(), context);
                        p.writePortable(classDefinitionWriter);
                        cd = classDefinitionWriter.getClassDefinition();
                        cd = context.registerClassDefinition(cd);
                    }

                    return cd;
                };

                template <typename T>
                void write(const T& p) {
                    util::AtomicPointer<ClassDefinition> cd = getClassDefinition(p);
                    PortableWriter portableWriter(context, cd, dataOutput);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                };

                int index;
                bool raw;
                DataOutput& dataOutput;
                SerializationContext& context;
                ObjectDataOutput& objectDataOutput;
                int begin;
                int offset;
                std::set<const char *, util::cStrCmp> writtenFields;
                util::AtomicPointer<ClassDefinition> cd;

            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_WRITER */
