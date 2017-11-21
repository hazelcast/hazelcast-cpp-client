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

#include <hazelcast/client/serialization/pimpl/SerializationConstants.h>
#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeInt(*(static_cast<const int32_t *>(object)));
                }

                void *IntegerSerializer::read(ObjectDataInput &in) {
                    return new int32_t(in.readInt());
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByte(*(static_cast<const byte *>(object)));
                }

                void *ByteSerializer::read(ObjectDataInput &in) {
                    return new byte(in.readByte());
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBoolean(*(static_cast<const bool *>(object)));
                }

                void *BooleanSerializer::read(ObjectDataInput &in) {
                    return new bool(in.readBoolean());
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeChar(*(static_cast<const char *>(object)));
                }

                void *CharSerializer::read(ObjectDataInput &in) {
                    return new char(in.readChar());
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShort(*(static_cast<const int16_t *>(object)));
                }

                void *ShortSerializer::read(ObjectDataInput &in) {
                    return new int16_t(in.readShort());
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLong(*(static_cast<const int64_t *>(object)));
                }

                void *LongSerializer::read(ObjectDataInput &in) {
                    return new int64_t(in.readLong());;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloat(*(static_cast<const float *>(object)));
                }

                void *FloatSerializer::read(ObjectDataInput &in) {
                    return new float(in.readFloat());
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDouble(*(static_cast<const double *>(object)));
                }

                void *DoubleSerializer::read(ObjectDataInput &in) {
                    return new double(in.readDouble());
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(static_cast<const std::string *>(object));
                }

                void *StringSerializer::read(ObjectDataInput &in) {
                    return in.readUTF().release();
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const void *object) {
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByteArray(static_cast<const std::vector<byte> *>(object));
                }

                void *TheByteArraySerializer::read(ObjectDataInput &in) {
                    return in.readByteArray().release();
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBooleanArray(static_cast<const std::vector<bool> *>(object));
                }

                void *BooleanArraySerializer::read(ObjectDataInput &in) {
                    return in.readBooleanArray().release();
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeCharArray(static_cast<const std::vector<char> *>(object));
                }

                void *CharArraySerializer::read(ObjectDataInput &in) {
                    return in.readCharArray().release();
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShortArray(static_cast<const std::vector<int16_t> *>(object));
                }

                void *ShortArraySerializer::read(ObjectDataInput &in) {
                    return in.readShortArray().release();
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeIntArray(static_cast<const std::vector<int32_t> *>(object));
                }

                void *IntegerArraySerializer::read(ObjectDataInput &in) {
                    return in.readIntArray().release();
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLongArray(static_cast<const std::vector<int64_t> *>(object));
                }

                void *LongArraySerializer::read(ObjectDataInput &in) {
                    return in.readLongArray().release();
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloatArray(static_cast<const std::vector<float> *>(object));
                }

                void *FloatArraySerializer::read(ObjectDataInput &in) {
                    return in.readFloatArray().release();
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDoubleArray(static_cast<const std::vector<double> *>(object));
                }

                void *DoubleArraySerializer::read(ObjectDataInput &in) {
                    return in.readDoubleArray().release();
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTFArray(static_cast<const std::vector<std::string *> *>(object));
                }

                void *StringArraySerializer::read(ObjectDataInput &in) {
                    return in.readUTFPointerArray().release();
                }
            }
        }
    }
}
