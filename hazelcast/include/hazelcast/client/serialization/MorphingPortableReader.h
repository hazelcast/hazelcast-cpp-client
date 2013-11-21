//
//  MorphingPortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_MORPHING_PORTABLE_READER
#define HAZELCAST_MORPHING_PORTABLE_READER

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/DataInput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include <string>
#include <memory>

namespace hazelcast {
    namespace client {

        class Portable;

        namespace serialization {

            class SerializerHolder;

            typedef unsigned char byte;

            class MorphingPortableReader {
            public:

                MorphingPortableReader(SerializationContext& serializationContext, DataInput& input, ClassDefinition *cd);

                int readInt(const char *fieldName);

                long readLong(const char *fieldName);

                bool readBoolean(const char *fieldName);

                byte readByte(const char *fieldName);

                char readChar(const char *fieldName);

                double readDouble(const char *fieldName);

                float readFloat(const char *fieldName);

                short readShort(const char *fieldName);

                std::string readUTF(const char *fieldName);

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
                    if (setPosition(fieldName))
                        return portable;
                    bool isNull = dataInput.readBoolean();
                    if (isNull) {
                        return portable;
                    }
                    read(dataInput, portable, currentFactoryId, currentClassId, cd->getVersion());
                    return portable;
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                    std::vector< T > portables;
                    if (setPosition(fieldName))
                        return portables;
                    int len = dataInput.readInt();
                    portables.resize(len, T());
                    if (len > 0) {
                        int offset = dataInput.position();
                        for (int i = 0; i < len; i++) {
                            dataInput.position(offset + i * sizeof (int));
                            int start = dataInput.readInt();
                            dataInput.position(start);
                            serializerHolder.getPortableSerializer().read(dataInput, portables[i], currentFactoryId, currentClassId, cd->getVersion());
                        }
                    }
                    return portables;
                };

                void end();

                ObjectDataInput& getRawDataInput();

            private:
                void read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion);

                int getPosition(const char *);

                bool setPosition(const char *);

                SerializationContext& context;
                SerializerHolder& serializerHolder;
                DataInput& dataInput;
                ObjectDataInput objectDataInput;
                int const finalPosition;
                int offset;
                bool raw;
                ClassDefinition *cd;

                FieldType currentFieldType;
                int currentFactoryId;
                int currentClassId;
            };


        }
    }
}
#endif /* HAZELCAST_MORPHING_PORTABLE_READER */

