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
// Created by sancar koyunlu on 8/12/13.

#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput &dataOutput, pimpl::SerializerHolder *serializerHolder)
                    : dataOutput(&dataOutput), serializerHolder(serializerHolder), isEmpty(false) {

            }

            ObjectDataOutput::ObjectDataOutput()
                    : dataOutput(NULL), serializerHolder(NULL), isEmpty(true) {
            }

            std::auto_ptr<std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return std::auto_ptr<std::vector<byte> >((std::vector<byte> *) NULL);
                return dataOutput->toByteArray();
            }

            void ObjectDataOutput::write(const std::vector<byte> &bytes) {
                if (isEmpty) return;
                dataOutput->write(bytes);
            }

            void ObjectDataOutput::writeBoolean(bool i) {
                if (isEmpty) return;
                dataOutput->writeBoolean(i);
            }

            void ObjectDataOutput::writeByte(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeByte(i);
            }

            void ObjectDataOutput::writeBytes(const byte *bytes, size_t len) {
                if (isEmpty) return;
                dataOutput->writeBytes(bytes, len);
            }

            void ObjectDataOutput::writeShort(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeShort(v);
            }

            void ObjectDataOutput::writeChar(int32_t i) {
                if (isEmpty) return;
                dataOutput->writeChar(i);
            }

            void ObjectDataOutput::writeInt(int32_t v) {
                if (isEmpty) return;
                dataOutput->writeInt(v);
            }

            void ObjectDataOutput::writeLong(int64_t l) {
                if (isEmpty) return;
                dataOutput->writeLong(l);
            }

            void ObjectDataOutput::writeFloat(float x) {
                if (isEmpty) return;
                dataOutput->writeFloat(x);
            }

            void ObjectDataOutput::writeDouble(double v) {
                if (isEmpty) return;
                dataOutput->writeDouble(v);
            }

            void ObjectDataOutput::writeUTF(const std::string *str) {
                if (isEmpty) return;

                if (NULL == str) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    dataOutput->writeUTF(str);
                }
            }

            void ObjectDataOutput::writeByteArray(const std::vector<byte> *value) {
                if (isEmpty) return;
                dataOutput->writeByteArray(value);
            }

            void ObjectDataOutput::writeCharArray(const std::vector<char> *data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            }

            void ObjectDataOutput::writeBooleanArray(const std::vector<bool> *data) {
                if (isEmpty) return;
                dataOutput->writeBooleanArray(data);
            }

            void ObjectDataOutput::writeShortArray(const std::vector<int16_t> *data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            }

            void ObjectDataOutput::writeIntArray(const std::vector<int32_t> *data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            }

            void ObjectDataOutput::writeLongArray(const std::vector<int64_t > *data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            }

            void ObjectDataOutput::writeFloatArray(const std::vector<float> *data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            }

            void ObjectDataOutput::writeDoubleArray(const std::vector<double> *data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            }

            void ObjectDataOutput::writeUTFArray(const std::vector<std::string *> *strings) {
                if (isEmpty) return;

                int32_t len = NULL != strings ? (int32_t) strings->size() : util::Bits::NULL_ARRAY;

                writeInt(len);

                if (len > 0) {
                    for (std::vector<std::string *>::const_iterator it = strings->begin(); it != strings->end(); ++it) {
                        writeUTF(*it);
                    }
                }
            }

            void ObjectDataOutput::writeStringArray(const std::vector<std::string> *strings) {
                if (isEmpty) return;

                int32_t len = NULL != strings ? (int32_t) strings->size() : util::Bits::NULL_ARRAY;

                writeInt(len);

                if (len > 0) {
                    for (std::vector<std::string>::const_iterator it = strings->begin(); it != strings->end(); ++it) {
                        writeUTF(&(*it));
                    }
                }
            }

            void ObjectDataOutput::writeData(const pimpl::Data *data) {
                if (NULL == data || 0 == data->dataSize()) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    writeByteArray(&data->toByteArray());
                }

            }

            size_t ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(size_t newPos) {
                dataOutput->position(newPos);
            }

            pimpl::DataOutput *ObjectDataOutput::getDataOutput() const {
                return dataOutput;
            }
        }
    }
}

