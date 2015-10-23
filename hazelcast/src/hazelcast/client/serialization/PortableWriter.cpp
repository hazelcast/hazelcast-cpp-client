/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
            :defaultPortableWriter(defaultPortableWriter)
            , classDefinitionWriter(NULL)
            , isDefaultWriter(true) {

            }

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
            :defaultPortableWriter(NULL)
            , classDefinitionWriter(classDefinitionWriter)
            , isDefaultWriter(false) {

            }

            void PortableWriter::writeInt(const char *fieldName, int value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeInt(fieldName, value);
                return classDefinitionWriter->writeInt(fieldName, value);
            }

            void PortableWriter::writeLong(const char *fieldName, long value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLong(fieldName, value);
                return classDefinitionWriter->writeLong(fieldName, value);
            }

            void PortableWriter::writeBoolean(const char *fieldName, bool value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeBoolean(fieldName, value);
                return classDefinitionWriter->writeBoolean(fieldName, value);
            }

            void PortableWriter::writeByte(const char *fieldName, byte value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByte(fieldName, value);
                return classDefinitionWriter->writeByte(fieldName, value);
            }

            void PortableWriter::writeChar(const char *fieldName, int value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeChar(fieldName, value);
                return classDefinitionWriter->writeChar(fieldName, value);
            }

            void PortableWriter::writeDouble(const char *fieldName, double value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDouble(fieldName, value);
                return classDefinitionWriter->writeDouble(fieldName, value);
            }

            void PortableWriter::writeFloat(const char *fieldName, float value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloat(fieldName, value);
                return classDefinitionWriter->writeFloat(fieldName, value);
            }

            void PortableWriter::writeShort(const char *fieldName, short value) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShort(fieldName, value);
                return classDefinitionWriter->writeShort(fieldName, value);
            }

            void PortableWriter::writeUTF(const char *fieldName, const std::string *str) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeUTF(fieldName, str);
                return classDefinitionWriter->writeUTF(fieldName, str);
            }

            void PortableWriter::writeByteArray(const char *fieldName, const std::vector<byte> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeByteArray(fieldName, data);
                return classDefinitionWriter->writeByteArray(fieldName, data);
            }

            void PortableWriter::writeCharArray(const char *fieldName, const std::vector<char > *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeCharArray(fieldName, data);
                return classDefinitionWriter->writeCharArray(fieldName, data);
            }

            void PortableWriter::writeShortArray(const char *fieldName, const std::vector<short > *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeShortArray(fieldName, data);
                return classDefinitionWriter->writeShortArray(fieldName, data);
            }

            void PortableWriter::writeIntArray(const char *fieldName, const std::vector<int> *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeIntArray(fieldName, data);
                return classDefinitionWriter->writeIntArray(fieldName, data);
            }

            void PortableWriter::writeLongArray(const char *fieldName, const std::vector<long > *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeLongArray(fieldName, data);
                return classDefinitionWriter->writeLongArray(fieldName, data);
            }

            void PortableWriter::writeFloatArray(const char *fieldName, const std::vector<float > *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeFloatArray(fieldName, data);
                return classDefinitionWriter->writeFloatArray(fieldName, data);
            }

            void PortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double > *data) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeDoubleArray(fieldName, data);
                return classDefinitionWriter->writeDoubleArray(fieldName, data);
            }

            void PortableWriter::end() {
                if (isDefaultWriter)
                    return defaultPortableWriter->end();
                return classDefinitionWriter->end();
            }

            ObjectDataOutput& PortableWriter::getRawDataOutput() {
                if (isDefaultWriter)
                    return defaultPortableWriter->getRawDataOutput();
                return classDefinitionWriter->getRawDataOutput();
            }

        }
    }
}


