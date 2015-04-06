//
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DEFAULT_PORTABLE_WRITER
#define HAZELCAST_DEFAULT_PORTABLE_WRITER

#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/FieldType.h"
#include "hazelcast/client/serialization/FieldDefinition.h"
#include "hazelcast/client/serialization/Portable.h"

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

            class ClassDefinition;

            namespace pimpl {

                class SerializerHolder;

                class PortableContext;

                class HAZELCAST_API DefaultPortableWriter  {
                public:

                    DefaultPortableWriter(PortableContext& portableContext, boost::shared_ptr<ClassDefinition> cd, DataOutput& output);

                    void writeInt(const char *fieldName, int value);

                    void writeLong(const char *fieldName, long value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, int value);

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
                        setPosition(fieldName, FieldTypes::TYPE_PORTABLE);
                        dataOutput.writeBoolean(true);

                        T obj;
                        Portable *p = (Portable *)(&obj);
                        dataOutput.writeInt(p->getFactoryId());
                        dataOutput.writeInt(p->getClassId());
                    }

                    template<typename T>
                    void writePortable(const char *fieldName, const T& portable) {
                        FieldDefinition const& fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE);
                        checkPortableAttributes(fd, portable);

                        dataOutput.writeBoolean(false);

                        dataOutput.writeInt(fd.getFactoryId());
                        dataOutput.writeInt(fd.getClassId());

                        write(portable);
                    }

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T>& values) {
                        FieldDefinition const& fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);
                        size_t len = values.size();
                        dataOutput.writeInt((int)len);

                        dataOutput.writeInt(fd.getFactoryId());
                        dataOutput.writeInt(fd.getClassId());

                        if (len > 0) {
                            size_t offset = dataOutput.position();
                            dataOutput.position(offset + len * util::Bits::INT_SIZE_IN_BYTES);
                            for (size_t i = 0; i < len; i++) {
                                Portable const& portable = values[i];
                                checkPortableAttributes(fd, portable);
                                size_t position = dataOutput.position();
                                dataOutput.writeInt((int)(offset + i * util::Bits::INT_SIZE_IN_BYTES), (int)position);
                                write(portable);
                            }
                        }
                    }

                    ObjectDataOutput& getRawDataOutput();

                private:

                    FieldDefinition const& setPosition(const char *fieldName, FieldType fieldType);

                    void write(const Portable& p);

                    void checkPortableAttributes(const FieldDefinition& fd, const Portable& portable);

                    bool raw;
                    SerializerHolder& serializerHolder;
                    DataOutput& dataOutput;
                    ObjectDataOutput objectDataOutput;
                    size_t begin;
                    size_t offset;
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

#endif /* HAZELCAST_DEFAULT_PORTABLE_WRITER */

