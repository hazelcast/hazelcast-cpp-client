//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantsMorphingPortableReader.h"
#include "MorphingPortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void readPortable(MorphingPortableReader& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            void readPortable(MorphingPortableReader& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            void readPortable(MorphingPortableReader& dataInput, char& data) {
                data = dataInput.readChar();
            };

            void readPortable(MorphingPortableReader& dataInput, short & data) {
                data = dataInput.readShort();
            };

            void readPortable(MorphingPortableReader& dataInput, int& data) {
                data = dataInput.readInt();
            };

            void readPortable(MorphingPortableReader& dataInput, long & data) {
                data = dataInput.readLong();
            };

            void readPortable(MorphingPortableReader& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            void readPortable(MorphingPortableReader& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            void readPortable(MorphingPortableReader& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<byte>& data) {
                data = dataInput.readByteArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<char >& data) {
                data = dataInput.readCharArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<short >& data) {
                data = dataInput.readShortArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<int>& data) {
                data = dataInput.readIntArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<long >& data) {
                data = dataInput.readLongArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<float >& data) {
                data = dataInput.readFloatArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<double >& data) {
                data = dataInput.readDoubleArray();
            };

            void operator >>(MorphingPortableReader& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            void operator >>(MorphingPortableReader& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            void operator >>(MorphingPortableReader& dataInput, char& data) {
                data = dataInput.readChar();
            };

            void operator >>(MorphingPortableReader& dataInput, short & data) {
                data = dataInput.readShort();
            };

            void operator >>(MorphingPortableReader& dataInput, int& data) {
                data = dataInput.readInt();
            };

            void operator >>(MorphingPortableReader& dataInput, long & data) {
                data = dataInput.readLong();
            };

            void operator >>(MorphingPortableReader& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            void operator >>(MorphingPortableReader& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            void operator >>(MorphingPortableReader& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<byte>& data) {
                data = dataInput.readByteArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<char >& data) {
                data = dataInput.readCharArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<short >& data) {
                data = dataInput.readShortArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<int>& data) {
                data = dataInput.readIntArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<long >& data) {
                data = dataInput.readLongArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<float >& data) {
                data = dataInput.readFloatArray();
            };

            void operator >>(MorphingPortableReader& dataInput, std::vector<double >& data) {
                data = dataInput.readDoubleArray();
            };

        }
    }
}
