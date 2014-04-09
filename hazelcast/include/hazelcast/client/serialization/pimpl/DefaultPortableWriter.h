//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include <string>
#include <set>
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;
            namespace pimpl {
                class ClassDefinition;

                class SerializerHolder;

                class SerializationContext;

                class HAZELCAST_API DefaultPortableWriter {
                public:

                    DefaultPortableWriter(SerializationContext &serializationContext, boost::shared_ptr<ClassDefinition> cd, DataOutput &output);

                    void writeInt(const char *fieldName, int value);

                    void writeLong(const char *fieldName, long value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, short value);

                    void writeUTF(const char *fieldName, const std::string &str);

                    void writeNullPortable(const char *fieldName, int factoryId, int classId);

                    void writeByteArray(const char *fieldName, const std::vector<byte> &x);

                    void writeCharArray(const char *fieldName, const std::vector<char > &data);

                    void writeShortArray(const char *fieldName, const std::vector<short > &data);

                    void writeIntArray(const char *fieldName, const std::vector<int> &data);

                    void writeLongArray(const char *fieldName, const std::vector<long > &data);

                    void writeFloatArray(const char *fieldName, const std::vector<float > &data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double > &data);

                    void end();

                    template <typename T>
                    void writePortable(const char *fieldName, const T &portable) {
                        setPosition(fieldName);
                        dataOutput.writeBoolean(false);
                        write(portable);
                    };

                    template <typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> &values) {
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

                    ObjectDataOutput &getRawDataOutput();

                private:

                    void setPosition(const char *fieldName);

                    void write(const Portable &p);

                    bool raw;
                    SerializerHolder &serializerHolder;
                    DataOutput &dataOutput;
                    ObjectDataOutput objectDataOutput;
                    int begin;
                    int offset;
                    std::set<std::string> writtenFields;
                    boost::shared_ptr<ClassDefinition> cd;

                };
            }
        }
    }
}
#endif /* HAZELCAST_PORTABLE_WRITER */

