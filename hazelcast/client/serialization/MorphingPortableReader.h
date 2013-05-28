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
#include "BufferedDataInput.h"
#include "FieldDefinition.h"
#include "../HazelcastException.h"
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

                MorphingPortableReader(SerializationContext *serializationContext, BufferedDataInput& input, ClassDefinition *cd);

                MorphingPortableReader& operator [](std::string fieldName);

                int skipBytes(int i);

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
                void read(BufferedDataInput& dataInput, T& object, int factoryId, int classId, int dataVersion) {

//                    PortableFactory const *portableFactory;
//                    if (portableFactories.count(factoryId) != 0) {
//                        portableFactory = portableFactories.at(factoryId);
//                    } else {
//                        throw hazelcast::client::HazelcastException("Could not find PortableFactory for factoryId: " + hazelcast::client::util::to_string(factoryId));
//                    }
//
//                    std::auto_ptr<Portable> p(portableFactory->create(classId));
//                    if (p.get() == NULL) {
//                        throw hazelcast::client::HazelcastException("Could not create Portable for class-id: " + hazelcast::client::util::to_string(factoryId));
//                    }

                    ClassDefinition *cd;
                    if (context->getVersion() == dataVersion) {
                        cd = context->lookup(factoryId, classId); // using serializationContext.version
                        PortableReader reader(context, dataInput, cd);
                        hazelcast::client::serialization::readPortable(reader, object);
                    } else {
                        cd = context->lookup(factoryId, classId, dataVersion); // registered during read
                        MorphingPortableReader reader(context, dataInput, cd);
                        hazelcast::client::serialization::readPortable(reader, object);
                    }
                };

                template<typename T>
                void readPortable(T& portable) {
                    if (!cd->isFieldDefinitionExists(lastFieldName))
                        return;
                    FieldDefinition fd = cd->get(lastFieldName);
                    bool isNull = input.readBoolean();
                    if (isNull) {
                        return;
                    }
                    read(input, portable, fd.getFactoryId(), fd.getClassId(), cd->getVersion());
                };

                template<typename T>
                void readPortable(std::vector< T >& portables) {
                    if (!cd->isFieldDefinitionExists(lastFieldName))
                        return;
                    int len = input.readInt();
                    portables.resize(len, T());
                    if (len > 0) {
                        int offset = input.position();
                        for (int i = 0; i < len; i++) {
                            input.position(offset + i * sizeof (int));
                            int start = input.readInt();
                            input.position(start);
                            FieldDefinition fd = cd->get(lastFieldName);
                            read(input, portables[i], fd.getFactoryId(), fd.getClassId(), cd->getVersion());
                        }
                    }
                };

                void readingFromDataInput();

            private:

                int getPosition(std::string&);

                int offset;
                bool raw;
                bool readingPortable;
                SerializationContext *context;
                ClassDefinition *cd;
                BufferedDataInput& input;
                std::string lastFieldName;
            };

            template<typename T>
            inline void operator >>(MorphingPortableReader& portableReader, std::vector<T>& data) {
                portableReader.readingFromDataInput();
                portableReader.readPortable(data);
            };


            template<typename T>
            inline void operator >>(MorphingPortableReader& portableReader, T& data) {
                portableReader.readingFromDataInput();
                if (getTypeId(data) == SerializationConstants::CONSTANT_TYPE_PORTABLE)
                    portableReader.readPortable(data);
                else
                    readPortable(portableReader, data);
            };


        }
    }
}
#endif /* HAZELCAST_MORPHING_PORTABLE_READER */

