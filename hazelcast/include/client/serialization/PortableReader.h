//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_READER
#define HAZELCAST_PORTABLE_READER

#include "ClassDefinition.h"
#include "BufferedDataInput.h"
#include "FieldDefinition.h"
#include "../HazelcastException.h"
#include "ConstantSerializers.h"
#include "SerializationContext.h"

#include <iostream>
#include <string>
#include <memory>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {


            class BufferObjectDataInput;

            typedef unsigned char byte;

            class PortableReader {
            public:

                PortableReader(SerializationContext *serializationContext, BufferedDataInput& input, ClassDefinition *cd);

//                PortableReader& operator [](const std::string& fieldName);
                PortableReader& operator [](const char *fieldName);

                int skipBytes(int i);

                void readFully(std::vector<byte>&);

                int readInt();

                long readLong();

                bool readBoolean();

                byte readByte();

                char readChar();

                double readDouble();

                float readFloat();

                short readShort();

                string readUTF();

                std::vector<byte> readByteArray();

                std::vector<char> readCharArray();

                std::vector<int> readIntArray();

                std::vector<long> readLongArray();

                std::vector<double> readDoubleArray();

                std::vector<float> readFloatArray();

                std::vector<short> readShortArray();

                template<typename T>
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId) {

                    ClassDefinition *cd;
                    cd = context->lookup(factoryId, classId); // using serializationContext.version
                    PortableReader reader(context, dataInput, cd);
                    hazelcast::client::serialization::readPortable(reader, object);
                };

                template<typename T>
                void readPortable(T& portable) {
                    bool isNull = input.readBoolean();
                    if (isNull) {
                        return;
                    }
                    read(input, portable, currentFactoryId, currentClassId);
                };

                template<typename T>
                void readPortable(std::vector< T >& portables) {

                    int len = input.readInt();
                    portables.resize(len, T());
                    if (len > 0) {
                        int offset = input.position();
                        for (int i = 0; i < len; i++) {
                            input.position(offset + i * sizeof (int));
                            int start = input.readInt();
                            input.position(start);

                            read(input, portables[i], currentFactoryId, currentClassId);
                        }
                    }
                };

                void readingFromDataInput();

            private:

//                int getPosition(const std::string&);
                int getPosition(const char *);

                int offset;
                bool raw;
                bool readingPortable;
                SerializationContext *context;
                ClassDefinition *cd;
                BufferedDataInput& input;
                int currentFactoryId;
                int currentClassId;
            };

            template<typename T>
            inline void operator >>(PortableReader& portableReader, std::vector<T>& data) {
                portableReader.readingFromDataInput();
                portableReader.readPortable(data);
            };


            template<typename T>
            inline void operator >>(PortableReader& portableReader, T& data) {
                portableReader.readingFromDataInput();
                if (getTypeSerializerId(data) == SerializationConstants::CONSTANT_TYPE_PORTABLE)
                    portableReader.readPortable(data);
                else
                    readPortable(portableReader, data);
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_READER */
