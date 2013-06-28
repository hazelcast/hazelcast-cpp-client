//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantsMorphingPortableReader.h"
#include "hazelcast/client/serialization/MorphingPortableReader.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void operator >>(MorphingPortableReader& portableReader, byte& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readByte();
            };

            void operator >>(MorphingPortableReader& portableReader, bool& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readBoolean();
            };

            void operator >>(MorphingPortableReader& portableReader, char& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readChar();
            };

            void operator >>(MorphingPortableReader& portableReader, short & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readShort();
            };

            void operator >>(MorphingPortableReader& portableReader, int& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readInt();
            };

            void operator >>(MorphingPortableReader& portableReader, long & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readLong();
            };

            void operator >>(MorphingPortableReader& portableReader, float & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readFloat();
            };

            void operator >>(MorphingPortableReader& portableReader, double & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readDouble();
            };

            void operator >>(MorphingPortableReader& portableReader, std::string&  data) {
                portableReader.readingFromDataInput();
                data = portableReader.readUTF();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<byte>& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readByteArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<char >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readCharArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<short >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readShortArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<int>& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readIntArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<long >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readLongArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<float >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readFloatArray();
            };

            void operator >>(MorphingPortableReader& portableReader, std::vector<double >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readDoubleArray();
            };

            void operator >>(MorphingPortableReader& portableReader, Data& data) {
                throw HazelcastException("Not Supported");
            };

        }
    }
}
