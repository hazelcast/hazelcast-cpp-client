/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableReader::PortableReader(pimpl::PortableContext& context, ObjectDataInput& input, boost::shared_ptr<ClassDefinition> cd, bool isDefaultReader)
            : isDefaultReader(isDefaultReader) {
                if (isDefaultReader) {
                    defaultPortableReader.reset(new pimpl::DefaultPortableReader(context, input, cd));
                } else {
                    morphingPortableReader.reset(new pimpl::MorphingPortableReader(context, input, cd));
                }
            }

            PortableReader::PortableReader(const PortableReader& reader)
            : isDefaultReader(reader.isDefaultReader)
            , defaultPortableReader(reader.defaultPortableReader.release())
            , morphingPortableReader(reader.morphingPortableReader.release()) {

            }

            PortableReader& PortableReader::operator=(const PortableReader& reader) {
                this->isDefaultReader = reader.isDefaultReader;
                this->defaultPortableReader.reset(reader.defaultPortableReader.release());
                this->morphingPortableReader.reset(reader.morphingPortableReader.release());
                return *this;
            }

            int32_t PortableReader::readInt(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readInt(fieldName);
                return morphingPortableReader->readInt(fieldName);
            }

            int64_t PortableReader::readLong(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLong(fieldName);
                return morphingPortableReader->readLong(fieldName);
            }

            bool PortableReader::readBoolean(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBoolean(fieldName);
                return morphingPortableReader->readBoolean(fieldName);
            }

            byte PortableReader::readByte(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByte(fieldName);
                return morphingPortableReader->readByte(fieldName);
            }

            char PortableReader::readChar(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readChar(fieldName);
                return morphingPortableReader->readChar(fieldName);
            }

            double PortableReader::readDouble(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDouble(fieldName);
                return morphingPortableReader->readDouble(fieldName);
            }

            float PortableReader::readFloat(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloat(fieldName);
                return morphingPortableReader->readFloat(fieldName);
            }

            int16_t PortableReader::readShort(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShort(fieldName);
                return morphingPortableReader->readShort(fieldName);
            }

            std::auto_ptr<std::string> PortableReader::readUTF(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readUTF(fieldName);
                return morphingPortableReader->readUTF(fieldName);
            }

            std::auto_ptr<std::vector<byte> > PortableReader::readByteArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByteArray(fieldName);
                return morphingPortableReader->readByteArray(fieldName);
            }


            std::auto_ptr<std::vector<bool> > PortableReader::readBooleanArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBooleanArray(fieldName);
                return morphingPortableReader->readBooleanArray(fieldName);
            }

            std::auto_ptr<std::vector<char> > PortableReader::readCharArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readCharArray(fieldName);
                return morphingPortableReader->readCharArray(fieldName);
            }

            std::auto_ptr<std::vector<int32_t> > PortableReader::readIntArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readIntArray(fieldName);
                return morphingPortableReader->readIntArray(fieldName);
            }

            std::auto_ptr<std::vector<int64_t> > PortableReader::readLongArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLongArray(fieldName);
                return morphingPortableReader->readLongArray(fieldName);
            }

            std::auto_ptr<std::vector<double> > PortableReader::readDoubleArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDoubleArray(fieldName);
                return morphingPortableReader->readDoubleArray(fieldName);
            }

            std::auto_ptr<std::vector<float> > PortableReader::readFloatArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloatArray(fieldName);
                return morphingPortableReader->readFloatArray(fieldName);
            }

            std::auto_ptr<std::vector<int16_t> > PortableReader::readShortArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShortArray(fieldName);
                return morphingPortableReader->readShortArray(fieldName);
            }

            std::auto_ptr<std::vector<std::string> > PortableReader::readUTFArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readUTFArray(fieldName);
                return morphingPortableReader->readUTFArray(fieldName);
            }

			
            ObjectDataInput& PortableReader::getRawDataInput() {
                if (isDefaultReader)
                    return defaultPortableReader->getRawDataInput();
                return morphingPortableReader->getRawDataInput();
            }

            void PortableReader::end() {
                if (isDefaultReader)
                    return defaultPortableReader->end();
                return morphingPortableReader->end();

            }
        }
    }
}
