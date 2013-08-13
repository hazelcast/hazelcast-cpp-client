//
// Created by sancar koyunlu on 8/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_PortableWriter
#define HAZELCAST_PortableWriter

#include "DefaultPortableWriter.h"
#include "ClassDefinitionWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            class PortableWriter {
            public:
                PortableWriter(DefaultPortableWriter *defaultPortableWriter);

                PortableWriter(ClassDefinitionWriter *classDefinitionWriter);

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
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortable(fieldName, portable);
                    return classDefinitionWriter->writePortable(fieldName, portable);

                };

                template <typename T>
                void writePortableArray(const char *fieldName, const std::vector<T>& values) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortableArray(fieldName, values);
                    return classDefinitionWriter->writePortableArray(fieldName, values);
                };

                ObjectDataOutput& getRawDataOutput();

            private:
                bool isDefaultWriter;
                DefaultPortableWriter *defaultPortableWriter;
                ClassDefinitionWriter *classDefinitionWriter;
            };
        }
    }
}

#endif //HAZELCAST_PortableWriter
