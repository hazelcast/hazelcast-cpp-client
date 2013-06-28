//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantsPortableReader.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void operator >>(PortableReader& portableReader, byte& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readByte();
            };

            void operator >>(PortableReader& portableReader, bool& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readBoolean();
            };

            void operator >>(PortableReader& portableReader, char& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readChar();
            };

            void operator >>(PortableReader& portableReader, short & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readShort();
            };

            void operator >>(PortableReader& portableReader, int& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readInt();
            };

            void operator >>(PortableReader& portableReader, long & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readLong();
            };

            void operator >>(PortableReader& portableReader, float & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readFloat();
            };

            void operator >>(PortableReader& portableReader, double & data) {
                portableReader.readingFromDataInput();
                data = portableReader.readDouble();
            };

            void operator >>(PortableReader& portableReader, std::string&  data) {
                portableReader.readingFromDataInput();
                data = portableReader.readUTF();
            };

            void operator >>(PortableReader& portableReader, std::vector<byte>& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readByteArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<char >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readCharArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<short >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readShortArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<int>& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readIntArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<long >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readLongArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<float >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readFloatArray();
            };

            void operator >>(PortableReader& portableReader, std::vector<double >& data) {
                portableReader.readingFromDataInput();
                data = portableReader.readDoubleArray();
            };

            void operator >>(PortableReader& portableReader, Data& data) {
                throw HazelcastException("Not Supported");
            };

        }
    }
}
