//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantsMorphingPortableReader.h"
#include "../MorphingPortableReader.h"
#include "../Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void readPortable(MorphingPortableReader& portableReader, byte& data) {
                data = portableReader.readByte();
            };

            void readPortable(MorphingPortableReader& portableReader, bool& data) {
                data = portableReader.readBoolean();
            };

            void readPortable(MorphingPortableReader& portableReader, char& data) {
                data = portableReader.readChar();
            };

            void readPortable(MorphingPortableReader& portableReader, short & data) {
                data = portableReader.readShort();
            };

            void readPortable(MorphingPortableReader& portableReader, int& data) {
                data = portableReader.readInt();
            };

            void readPortable(MorphingPortableReader& portableReader, long & data) {
                data = portableReader.readLong();
            };

            void readPortable(MorphingPortableReader& portableReader, float & data) {
                data = portableReader.readFloat();
            };

            void readPortable(MorphingPortableReader& portableReader, double & data) {
                data = portableReader.readDouble();
            };

            void readPortable(MorphingPortableReader& portableReader, std::string&  data) {
                data = portableReader.readUTF();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<byte>& data) {
                data = portableReader.readByteArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<char >& data) {
                data = portableReader.readCharArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<short >& data) {
                data = portableReader.readShortArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<int>& data) {
                data = portableReader.readIntArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<long >& data) {
                data = portableReader.readLongArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<float >& data) {
                data = portableReader.readFloatArray();
            };

            void readPortable(MorphingPortableReader& portableReader, std::vector<double >& data) {
                data = portableReader.readDoubleArray();
            };

            void readPortable(MorphingPortableReader& portableReader, Data& data) {
                data.type = portableReader.readInt();
                int classId = portableReader.readInt();

                if (classId != data.NO_CLASS_ID) {
                    int factoryId = portableReader.readInt();
                    data.isError = (factoryId == hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                            && (classId == hazelcast::client::protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                    int version = portableReader.readInt();

                    int classDefSize = portableReader.readInt();

                    if (data.context->isClassDefinitionExists(factoryId, classId, version)) {
                        data.cd = data.context->lookup(factoryId, classId, version);
                        portableReader.skipBytes(classDefSize);
                    } else {
                        std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                        portableReader.readFully(*(classDefBytes.get()));
                        data.cd = data.context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = portableReader.readInt();
                if (size > 0) {
                    data.buffer->resize(size, 0);
                    portableReader.readFully(*(data.buffer.get()));
                }
                data.partitionHash = portableReader.readInt();

            };

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
                portableReader.readingFromDataInput();
                readPortable(portableReader, data);
            };

        }
    }
}
