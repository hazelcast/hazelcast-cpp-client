//
//  MorphingPortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_MORPHING_PORTABLE_READER
#define HAZELCAST_MORPHING_PORTABLE_READER

#include "ClassDefinition.h"
#include "ObjectDataInput.h"
#include "FieldDefinition.h"
#include "IException.h"
#include "PortableReader.h"
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

            class MorphingPortableReader {
            public:

                MorphingPortableReader(SerializationContext& serializationContext, ObjectDataInput& input, util::AtomicPointer<ClassDefinition> cd);

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
                void read(ObjectDataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {

                    util::AtomicPointer<ClassDefinition> cd;
                    if (context.getVersion() == dataVersion) {
                        cd = context.lookup(factoryId, classId); // using serializationContext.version
                        PortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                        reader.end();
                    } else {
                        cd = context.lookup(factoryId, classId, dataVersion); // registered during read
                        MorphingPortableReader reader(context, dataInput, cd);
                        object.readPortable(reader);
                        reader.end();
                    }
                };

                template<typename T>
                T readPortable(const char *fieldName) {
                    T portable;
                    if (setPosition(fieldName))
                        return portable;
                    bool isNull = input.readBoolean();
                    if (isNull) {
                        return portable;
                    }
                    read(input, portable, currentFactoryId, currentClassId, cd->getVersion());
                    return portable;
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                    std::vector< T > portables;
                    if (setPosition(fieldName))
                        return portables;
                    int len = input.readInt();
                    portables.resize(len, T());
                    if (len > 0) {
                        int offset = input.position();
                        for (int i = 0; i < len; i++) {
                            input.position(offset + i * sizeof (int));
                            int start = input.readInt();
                            input.position(start);
                            read(input, portables[i], currentFactoryId, currentClassId, cd->getVersion());
                        }
                    }
                    return portables;
                };

                void end();

                ObjectDataInput *getRawDataInput();

            private:

                int getPosition(const char *);

                bool setPosition(const char *);

                ObjectDataInput& input;
                int const finalPosition;
                int offset;
                bool raw;
                SerializationContext& context;
                util::AtomicPointer<ClassDefinition> cd;

                FieldType currentFieldType;
                int currentFactoryId;
                int currentClassId;
            };


        }
    }
}
#endif /* HAZELCAST_MORPHING_PORTABLE_READER */

