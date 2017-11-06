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
//  ObjectDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataInput::ObjectDataInput(pimpl::DataInput& dataInput, pimpl::SerializerHolder &serializerHolder)
            : dataInput(dataInput)
            , serializerHolder(serializerHolder) {
            }

            void ObjectDataInput::readFully(std::vector<byte>& bytes) {
                dataInput.readFully(bytes);
            }

            int ObjectDataInput::skipBytes(int i) {
                return dataInput.skipBytes(i);
            }

            bool ObjectDataInput::readBoolean() {
                return dataInput.readBoolean();
            }

            byte ObjectDataInput::readByte() {
                return dataInput.readByte();
            }

            int16_t ObjectDataInput::readShort() {
                return dataInput.readShort();
            }

            char ObjectDataInput::readChar() {
                return dataInput.readChar();
            }

            int32_t ObjectDataInput::readInt() {
                return dataInput.readInt();
            }

            int64_t ObjectDataInput::readLong() {
                return dataInput.readLong();
            }

            float ObjectDataInput::readFloat() {
                return dataInput.readFloat();
            }

            double ObjectDataInput::readDouble() {
                return dataInput.readDouble();
            }

            std::auto_ptr<std::string> ObjectDataInput::readUTF() {
                return dataInput.readUTF();
            }

            pimpl::Data ObjectDataInput::readData() {
                return pimpl::Data(dataInput.readByteArray());
            }

            int ObjectDataInput::position() {
                return dataInput.position();
            }

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            }

            std::auto_ptr<std::vector<byte> > ObjectDataInput::readByteArray() {
                return dataInput.readByteArray();
            }

            std::auto_ptr<std::vector<bool> > ObjectDataInput::readBooleanArray() {
                return dataInput.readBooleanArray();
            }

            std::auto_ptr<std::vector<char> > ObjectDataInput::readCharArray() {
                return dataInput.readCharArray();
            }

            std::auto_ptr<std::vector<int32_t> > ObjectDataInput::readIntArray() {
                return dataInput.readIntArray();
            }

            std::auto_ptr<std::vector<int64_t> > ObjectDataInput::readLongArray() {
                return dataInput.readLongArray();
            }

            std::auto_ptr<std::vector<double> > ObjectDataInput::readDoubleArray() {
                return dataInput.readDoubleArray();
            }

            std::auto_ptr<std::vector<float> > ObjectDataInput::readFloatArray() {
                return dataInput.readFloatArray();
            }

            std::auto_ptr<std::vector<int16_t> > ObjectDataInput::readShortArray() {
                return dataInput.readShortArray();
            }

            std::auto_ptr<std::vector<std::string> > ObjectDataInput::readUTFArray() {
                return dataInput.readUTFArray();
            }

            std::auto_ptr<std::vector<std::string *> > ObjectDataInput::readStringArray() {
                return dataInput.readStringArray();
            }

        }
    }
}

