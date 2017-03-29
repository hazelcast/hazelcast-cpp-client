/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

                    DefaultPortableWriter(PortableContext& portableContext, hazelcast::util::SharedPtr<ClassDefinition> cd, DataOutput& output);

                    void writeInt(const char *fieldName, int32_t value);

                    void writeLong(const char *fieldName, int64_t  value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int32_t value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, int32_t value);

                    void writeUTF(const char *fieldName, const std::string *str);

                    void writeByteArray(const char *fieldName, const std::vector<byte> *x);

                    void writeBooleanArray(const char *fieldName, const std::vector<bool> *x);

                    void writeCharArray(const char *fieldName, const std::vector<char> *data);

                    void writeShortArray(const char *fieldName, const std::vector<int16_t> *data);

                    void writeIntArray(const char *fieldName, const std::vector<int32_t> *data);

                    void writeLongArray(const char *fieldName, const std::vector<int64_t > *data);

                    void writeFloatArray(const char *fieldName, const std::vector<float> *data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double> *data);

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
                    void writePortable(const char *fieldName, const T *portable) {
                        FieldDefinition const& fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE);

                        bool isNull = (NULL == portable);
                        dataOutput.writeBoolean(isNull);

                        dataOutput.writeInt(fd.getFactoryId());
                        dataOutput.writeInt(fd.getClassId());

                        if (!isNull) {
                            checkPortableAttributes(fd, *portable);
                            write(*portable);
                        }
                    }

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> *values) {
                        FieldDefinition const& fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);
                        int32_t len =  (NULL == values ? util::Bits::NULL_ARRAY : (int32_t)values->size());
                        dataOutput.writeInt(len);

                        dataOutput.writeInt(fd.getFactoryId());
                        dataOutput.writeInt(fd.getClassId());

                        if (len > 0) {
                            size_t offset = dataOutput.position();
                            dataOutput.position(offset + len * util::Bits::INT_SIZE_IN_BYTES);
                            for (int32_t i = 0; i < len; i++) {
                                checkPortableAttributes(fd, (*values)[i]);
                                size_t position = dataOutput.position();
                                dataOutput.writeInt((int32_t)(offset + i * util::Bits::INT_SIZE_IN_BYTES), (int32_t)position);
                                write((*values)[i]);
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
                    hazelcast::util::SharedPtr<ClassDefinition> cd;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_DEFAULT_PORTABLE_WRITER */

