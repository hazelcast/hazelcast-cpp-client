//
//  ObjectDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ObjectDataInput.h"
#include "SerializationContext.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataInput::ObjectDataInput(DataInput & dataInput, SerializerHolder& serializerHolder, SerializationContext & context)
            : dataInput(dataInput)
            , serializationContext(context)
            , serializerHolder(serializerHolder) {

            }


            ObjectDataInput::ObjectDataInput(ObjectDataInput const & rhs)
            : dataInput(rhs.dataInput)
            , serializationContext(rhs.serializationContext)
            , serializerHolder(rhs.serializerHolder) {
                //private
            }

            void ObjectDataInput::operator = (ObjectDataInput const & param) {
                //private
            }

            SerializationContext *ObjectDataInput::getSerializationContext() {
                return &serializationContext;
            };

            void ObjectDataInput::readFully(std::vector<byte>& bytes) {
                dataInput.readFully(bytes);
            };

            int ObjectDataInput::skipBytes(int i) {
                dataInput.skipBytes(i);
            };

            bool ObjectDataInput::readBoolean() {
                return dataInput.readBoolean();
            };

            byte ObjectDataInput::readByte() {
                return dataInput.readByte();
            };

            short ObjectDataInput::readShort() {
                return dataInput.readShort();
            };

            char ObjectDataInput::readChar() {
                return dataInput.readChar();
            };

            int ObjectDataInput::readInt() {
                return dataInput.readInt();
            };

            long ObjectDataInput::readLong() {
                return dataInput.readLong();
            };

            float ObjectDataInput::readFloat() {
                return dataInput.readFloat();
            };

            double ObjectDataInput::readDouble() {
                return dataInput.readDouble();
            };

            std::string ObjectDataInput::readUTF() {
                return dataInput.readUTF();
            };

            int ObjectDataInput::position() {
                return dataInput.position();
            };

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            };

            std::vector <byte> ObjectDataInput::readByteArray() {
                return dataInput.readByteArray();
            };

            std::vector<char> ObjectDataInput::readCharArray() {
                return dataInput.readCharArray();
            };

            std::vector<int> ObjectDataInput::readIntArray() {
                return dataInput.readIntArray();
            };

            std::vector<long> ObjectDataInput::readLongArray() {
                return dataInput.readLongArray();
            };

            std::vector<double> ObjectDataInput::readDoubleArray() {
                return dataInput.readDoubleArray();
            };

            std::vector<float> ObjectDataInput::readFloatArray() {
                return dataInput.readFloatArray();
            };

            std::vector<short> ObjectDataInput::readShortArray() {
                int len = readInt();
                return dataInput.readShortArray();
            };

        }
    }
}
