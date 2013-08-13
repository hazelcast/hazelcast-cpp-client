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
#include "DataInput.h"
#include "FieldDefinition.h"
#include "IException.h"
#include "ConstantSerializers.h"
#include "SerializationContext.h"
#include "SerializationConstraints.h"
#include "ObjectDataInput.h"
#include <string>
#include <memory>

using namespace std;

namespace hazelcast {
    namespace client {

        class Portable;

        namespace serialization {

            class SerializerHolder;

            typedef unsigned char byte;

            class DefaultPortableReader {
            public:

                DefaultPortableReader(SerializerHolder& serializerHolder, SerializationContext& serializationContext, DataInput& input, util::AtomicPointer<ClassDefinition> cd);

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
                    Is_Portable<T>();
                    T portable;
                    setPosition(fieldName);
                    bool isNull = dataInput.readBoolean();
                    if (isNull) {
                        return portable;
                    }
                    read(dataInput, portable, currentFactoryId, currentClassId);
                    return portable;
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                    Is_Portable<T>();
                    std::vector< T > portables;
                    setPosition(fieldName);
                    int len = dataInput.readInt();
                    portables.resize(len, T());
                    if (len > 0) {
                        int offset = dataInput.position();
                        for (int i = 0; i < len; i++) {
                            dataInput.position(offset + i * sizeof (int));
                            int start = dataInput.readInt();
                            dataInput.position(start);

                            read(dataInput, portables[i], currentFactoryId, currentClassId);
                        }
                    }
                    return portables;
                };

                ObjectDataInput& getRawDataInput();

                void end();

            private:
                int getPosition(const char *);

                void setPosition(const char *);

                void read(DataInput& dataInput, Portable& object, int factoryId, int classId);

                SerializerHolder& serializerHolder;
                SerializationContext& context;
                DataInput& dataInput;
                ObjectDataInput objectDataInput;
                int const finalPosition;
                int offset;
                bool raw;
                util::AtomicPointer<ClassDefinition> cd;
                int currentFactoryId;
                int currentClassId;
            };


        }
    }
}
#endif /* HAZELCAST_PORTABLE_READER */
