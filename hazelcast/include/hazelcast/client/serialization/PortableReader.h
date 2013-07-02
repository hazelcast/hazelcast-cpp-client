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

                int readInt(const char *fieldName);

                long readLong(const char *fieldName);

                bool readBoolean(const char *fieldName);

                byte readByte(const char *fieldName);

                char readChar(const char *fieldName);

                double readDouble(const char *fieldName);

                float readFloat(const char *fieldName);

                short readShort(const char *fieldName);

                string readUTF(const char *fieldName);

                std::vector<byte> readByteArray(const char *fieldName);

                std::vector<char> readCharArray(const char *fieldName);

                std::vector<int> readIntArray(const char *fieldName);

                std::vector<long> readLongArray(const char *fieldName);

                std::vector<double> readDoubleArray(const char *fieldName);

                std::vector<float> readFloatArray(const char *fieldName);

                std::vector<short> readShortArray(const char *fieldName);

                template<typename T>
                T readPortable(const char *fieldName) {
                    T portable;
                    setPosition(fieldName);
                    bool isNull = input.readBoolean();
                    if (isNull) {
                        return portable;
                    }
                    read(input, portable, currentFactoryId, currentClassId);
                    return portable;
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                    std::vector< T > portables;
                    setPosition(fieldName);
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
                    return portables;
                };

                BufferedDataInput *getRawDataInput();

            private:
                int getPosition(const char *);

                void setPosition(const char *);

                template<typename T>
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId) {
                    ClassDefinition *cd;
                    cd = context->lookup(factoryId, classId); // using serializationContext.version
                    PortableReader reader(context, dataInput, cd);
                    hazelcast::client::serialization::readPortable(reader, object);
                };

                int offset;
                bool raw;
                SerializationContext *context;
                ClassDefinition *cd;
                BufferedDataInput& input;
                int currentFactoryId;
                int currentClassId;
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_READER */
