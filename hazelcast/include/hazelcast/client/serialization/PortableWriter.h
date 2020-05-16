/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 8/10/13.

#pragma once
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            /**
            * Provides a mean of writing portable fields to a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableWriter {
            public:
                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter);

                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter);

                /**
                * @param fieldName name of the field
                * @param value     int32_t value to be written
                * @throws IOException
                */
                void writeInt(const char *fieldName, int32_t value);

                /**
                * @param fieldName name of the field
                * @param value int64_t value to be written
                * @throws IOException
                */
                void writeLong(const char *fieldName, int64_t value);

                /**
                * @param fieldName name of the field
                * @param value boolean value to be written
                * @throws IOException
                */
                void writeBoolean(const char *fieldName, bool value);

                /**
                * @param fieldName name of the field
                * @param value byte value to be written
                * @throws IOException
                */
                void writeByte(const char *fieldName, byte value);

                /**
                * @param fieldName name of the field
                * @param value  char value to be written
                * @throws IOException
                */
                void writeChar(const char *fieldName, int32_t value);

                /**
                * @param fieldName name of the field
                * @param value double value to be written
                * @throws IOException
                */
                void writeDouble(const char *fieldName, double value);

                /**
                * @param fieldName name of the field
                * @param value float value to be written
                * @throws IOException
                */
                void writeFloat(const char *fieldName, float value);

                /**
                * @param fieldName name of the field
                * @param value int16_t value to be written
                * @throws IOException
                */
                void writeShort(const char *fieldName, int16_t value);

                /**
                * @param fieldName name of the field
                * @param value utf string value to be written
                * @throws IOException
                */
                void writeUTF(const char *fieldName, const std::string *value);

                /**
                * @param fieldName name of the field
                * @param values byte array to be written
                * @throws IOException
                */
                void writeByteArray(const char *fieldName, const std::vector<byte> *values);

                /**
                * @param fieldName name of the field
                * @param values bool array to be written
                * @throws IOException
                */
                void writeBooleanArray(const char *fieldName, const std::vector<bool> *values);

                /**
                * @param fieldName name of the field
                * @param values     char array to be written
                * @throws IOException
                */
                void writeCharArray(const char *fieldName, const std::vector<char> *values);

                /**
                * @param fieldName name of the field
                * @param values int16_t array to be written
                * @throws IOException
                */
                void writeShortArray(const char *fieldName, const std::vector<int16_t> *values);

                /**
                * @param fieldName name of the field
                * @param values int32_t array to be written
                * @throws IOException
                */
                void writeIntArray(const char *fieldName, const std::vector<int32_t> *values);

                /**
                * @param fieldName name of the field
                * @param values     int64_t array to be written
                * @throws IOException
                */
                void writeLongArray(const char *fieldName, const std::vector<int64_t> *values);

                /**
                * @param fieldName name of the field
                * @param values    float array to be written
                * @throws IOException
                */
                void writeFloatArray(const char *fieldName, const std::vector<float> *values);

                /**
                * @param fieldName name of the field
                * @param values    double array to be written
                * @throws IOException
                */
                void writeDoubleArray(const char *fieldName, const std::vector<double> *values);

                /**
                * Internal api , should not be called by end user.
                */
                void end();

                /**
                * To write a null portable value.
                *
                * @tparam type of the portable field
                * @param fieldName name of the field
                *
                * @throws IOException
                */
                template<typename T>
                void writeNullPortable(const char *fieldName) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writeNullPortable<T>(fieldName);
                    return classDefinitionWriter->writeNullPortable<T>(fieldName);
                }

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param portable  Portable to be written
                * @throws IOException
                */
                template<typename T>
                void writePortable(const char *fieldName, const T *portable) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortable(fieldName, portable);
                    return classDefinitionWriter->writePortable(fieldName, portable);

                };

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param values portable array to be written
                * @throws IOException
                */
                template<typename T>
                void writePortableArray(const char *fieldName, const std::vector<T> *values) {
                    if (isDefaultWriter)
                        return defaultPortableWriter->writePortableArray(fieldName, values);
                    return classDefinitionWriter->writePortableArray(fieldName, values);
                };

                /**
                * After writing portable fields, one can write remaining fields in old fashioned way consecutively at the end
                * of stream. User should not that after getting rawDataOutput trying to write portable fields will result
                * in IOException
                *
                * @return ObjectDataOutput
                * @throws IOException
                */
                ObjectDataOutput &getRawDataOutput();

            private:
                pimpl::DefaultPortableWriter *defaultPortableWriter;
                pimpl::ClassDefinitionWriter *classDefinitionWriter;
                bool isDefaultWriter;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


