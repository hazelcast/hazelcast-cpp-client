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

                void IntegerSerializer::write(ObjectDataOutput &out, const int32_t &object) {
                    out.writeInt(object);
                }

                void IntegerSerializer::read(ObjectDataInput &in, int32_t &object) {
                    object = in.readInt();
                }

                void *IntegerSerializer::create(ObjectDataInput &in) {
                    return new int32_t;
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const byte &object) {
                    out.writeByte(object);
                }

                void ByteSerializer::read(ObjectDataInput &in, byte &object) {
                    object = in.readByte();
                }

                void *ByteSerializer::create(ObjectDataInput &in) {
                    return new byte;
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const bool &object) {
                    out.writeBoolean(object);
                }

                void BooleanSerializer::read(ObjectDataInput &in, bool &object) {
                    object = in.readBoolean();
                }

                void *BooleanSerializer::create(ObjectDataInput &in) {
                    return new bool;
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const char &object) {
                    out.writeChar(object);
                }

                void CharSerializer::read(ObjectDataInput &in, char &object) {
                    object = in.readChar();
                }

                void *CharSerializer::create(ObjectDataInput &in) {
                    return new char;
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const int16_t &object) {
                    out.writeShort(object);
                }

                void ShortSerializer::read(ObjectDataInput &in, int16_t &object) {
                    object = in.readShort();
                }

                void *ShortSerializer::create(ObjectDataInput &in) {
                    return new int16_t;
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const int64_t &object) {
                    out.writeLong(object);
                }

                void LongSerializer::read(ObjectDataInput &in, int64_t &object) {
                    object = in.readLong();
                }

                void *LongSerializer::create(ObjectDataInput &in) {
                    return new int64_t;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const float &object) {
                    out.writeFloat(object);
                }

                void FloatSerializer::read(ObjectDataInput &in, float &object) {
                    object = in.readFloat();
                }

                void *FloatSerializer::create(ObjectDataInput &in) {
                    return new float;
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const double &object) {
                    out.writeDouble(object);
                }

                void DoubleSerializer::read(ObjectDataInput &in, double &object) {
                    object = in.readDouble();
                }

                void *DoubleSerializer::create(ObjectDataInput &in) {
                    return new double;
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const std::string &object) {
                    out.writeUTF(&object);
                }

                void StringSerializer::read(ObjectDataInput &in, std::string &object) {
                    object = *in.readUTF();
                }

                void *StringSerializer::create(ObjectDataInput &in) {
                    return new std::string;
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const byte &object) {
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const std::vector<byte> &object) {
                    out.writeByteArray(&object);
                }

                void TheByteArraySerializer::read(ObjectDataInput &in, std::vector<byte> &object) {
                    object = *in.readByteArray();
                }

                void *TheByteArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<byte>;
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const std::vector<bool> &object) {
                    out.writeBooleanArray(&object);
                }

                void BooleanArraySerializer::read(ObjectDataInput &in, std::vector<bool> &object) {
                    object = *in.readBooleanArray();
                }

                void *BooleanArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<bool>;
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const std::vector<char> &object) {
                    out.writeCharArray(&object);
                }

                void CharArraySerializer::read(ObjectDataInput &in, std::vector<char> &object) {
                    object = *in.readCharArray();
                }

                void *CharArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<char>;
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const std::vector<int16_t> &object) {
                    out.writeShortArray(&object);
                }

                void ShortArraySerializer::read(ObjectDataInput &in, std::vector<int16_t> &object) {
                    object = *in.readShortArray();
                }

                void *ShortArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<int16_t>;
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const std::vector<int32_t> &object) {
                    out.writeIntArray(&object);
                }

                void IntegerArraySerializer::read(ObjectDataInput &in, std::vector<int32_t> &object) {
                    object = *in.readIntArray();
                }

                void *IntegerArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<int32_t>;
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const std::vector<int64_t> &object) {
                    out.writeLongArray(&object);
                }

                void LongArraySerializer::read(ObjectDataInput &in, std::vector<int64_t> &object) {
                    object = *in.readLongArray();
                }

                void *LongArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<int64_t>;
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const std::vector<float> &object) {
                    out.writeFloatArray(&object);
                }

                void FloatArraySerializer::read(ObjectDataInput &in, std::vector<float> &object) {
                    object = *in.readFloatArray();
                }

                void *FloatArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<float>;
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const std::vector<double> &object) {
                    out.writeDoubleArray(&object);
                }

                void DoubleArraySerializer::read(ObjectDataInput &in, std::vector<double> &object) {
                    object = *in.readDoubleArray();
                }

                void *DoubleArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<double>;
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const std::vector<std::string> &object) {
                    out.writeStringArray(&object);
                }

                void StringArraySerializer::read(ObjectDataInput &in, std::vector<std::string> &object) {
                    object = *in.readUTFArray();
                }

                void *StringArraySerializer::create(ObjectDataInput &in) {
                    return new std::vector<std::string>;
                }
            }
        }
    }
}
