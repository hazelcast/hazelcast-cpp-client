//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantsPortableReader.h"
#include "PortableReader.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {


            void readPortable(PortableReader& portableReader, byte& data) {
                data = portableReader.readByte();
            };

            void readPortable(PortableReader& portableReader, bool& data) {
                data = portableReader.readBoolean();
            };

            void readPortable(PortableReader& portableReader, char& data) {
                data = portableReader.readChar();
            };

            void readPortable(PortableReader& portableReader, short & data) {
                data = portableReader.readShort();
            };

            void readPortable(PortableReader& portableReader, int& data) {
                data = portableReader.readInt();
            };

            void readPortable(PortableReader& portableReader, long & data) {
                data = portableReader.readLong();
            };

            void readPortable(PortableReader& portableReader, float & data) {
                data = portableReader.readFloat();
            };

            void readPortable(PortableReader& portableReader, double & data) {
                data = portableReader.readDouble();
            };

            void readPortable(PortableReader& portableReader, std::string&  data) {
                data = portableReader.readUTF();
            };

            void readPortable(PortableReader& portableReader, std::vector<byte>& data) {
                data = portableReader.readByteArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<char >& data) {
                data = portableReader.readCharArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<short >& data) {
                data = portableReader.readShortArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<int>& data) {
                data = portableReader.readIntArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<long >& data) {
                data = portableReader.readLongArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<float >& data) {
                data = portableReader.readFloatArray();
            };

            void readPortable(PortableReader& portableReader, std::vector<double >& data) {
                data = portableReader.readDoubleArray();
            };
            
            void readPortable(PortableReader& portableReader , Data& data){                            
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
                            std::auto_ptr< std::vector<byte>> classDefBytes (new std::vector<byte> (classDefSize));
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
                portableReader.readingFromDataInput();
                readPortable(portableReader, data);
            };

        }
    }
}
