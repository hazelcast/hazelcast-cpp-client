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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            class ClassDefinition;
            
            class FieldDefinition;

            namespace pimpl {

                class SerializerHolder;

                class PortableContext;

                class HAZELCAST_API DefaultPortableWriter {
                public:

                    DefaultPortableWriter(PortableContext& portableContext, boost::shared_ptr<ClassDefinition> cd, DataOutput& output);

                    void writeInt(const char *fieldName, int value);

                    void writeLong(const char *fieldName, long value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, short value);

                    void writeUTF(const char *fieldName, const std::string& str);

                    void writeByteArray(const char *fieldName, const std::vector<byte>& x);

                    void writeCharArray(const char *fieldName, const std::vector<char>& data);

                    void writeShortArray(const char *fieldName, const std::vector<short>& data);

                    void writeIntArray(const char *fieldName, const std::vector<int>& data);

                    void writeLongArray(const char *fieldName, const std::vector<long>& data);

                    void writeFloatArray(const char *fieldName, const std::vector<float>& data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double>& data);

                    void end();

                    template<typename T>
                    void writeNullPortable(const char *fieldName) {
                        setPosition(fieldName);
                        dataOutput.writeBoolean(true);
                    }

                    template<typename T>
                    void writePortable(const char *fieldName, const T& portable) {
                        FieldDefinition const& fieldDefinition = setPosition(fieldName);
                        checkPortableAttributes(fieldDefinition, portable);
                        dataOutput.writeBoolean(false);
                        checkPortableAttributes(fieldDefinition, portable);
                        write(portable);
                    }

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T>& values) {
                        FieldDefinition const& fieldDefinition = setPosition(fieldName);
                        int len = values.size();
                        dataOutput.writeInt(len);
                        if (len > 0) {
                            int offset = dataOutput.position();
                            dataOutput.position(offset + len * sizeof(int));
                            for (int i = 0; i < len; i++) {
                                dataOutput.writeInt(offset + i * sizeof(int), dataOutput.position());
                                Portable const& portable = values[i];
                                checkPortableAttributes(fieldDefinition, portable);
                                write(portable);
                            }
                        }
                    }

                    ObjectDataOutput& getRawDataOutput();

                private:

                    FieldDefinition const& setPosition(const char *fieldName);

                    void write(const Portable& p);

                    void checkPortableAttributes(const FieldDefinition& fd, const Portable& portable);

                    bool raw;
                    SerializerHolder& serializerHolder;
                    DataOutput& dataOutput;
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_PORTABLE_WRITER */

