//
//  ObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "IException.h"
#include "ConstantSerializers.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializationService;

            class SerializationContext;

            class SerializerHolder;

            typedef unsigned char byte;

            class ObjectDataInput {
            public:
                ObjectDataInput(const std::vector<byte>& rhsBuffer);

                ObjectDataInput(const std::vector<byte>&, SerializerHolder&, SerializationContext&);

                void setSerializationContext(SerializationContext* context);

                SerializationContext * getSerializationContext();

                void readFully(std::vector<byte>&);

                int skipBytes(int i);

                bool readBoolean();

                byte readByte();

                short readShort();

                char readChar();

                int readInt();

                long readLong();

                float readFloat();

                double readDouble();

                std::string readUTF();

                std::vector<byte> readByteArray();

                std::vector<char> readCharArray();

                std::vector<int> readIntArray();

                std::vector<long> readLongArray();

                std::vector<double> readDoubleArray();

                std::vector<float> readFloatArray();

                std::vector<short> readShortArray();
                /*
                template<typename  T>
                T readObject(ObjectDataInput& input) {
                    T *tag;
                    return readObjectResolved<T>(input, tag);
                };

                template<typename  T>
                T readObjectResolved(ObjectDataInput& input, Portable *tag) {
                    bool isNull = input.readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = input.readInt();

                    ClassDefinition classDefinition;
                    classDefinition.readData(input);
                    int factoryId = classDefinition.getFactoryId();
                    int classId = classDefinition.getClassId();
                    int version = classDefinition.getVersion();

                    serializerHolder->getPortableSerializer().read(input, object, factoryId, classId, version);
                };

                template<typename  T>
                T readObjectResolved(ObjectDataInput& input, DataSerializable *tag) {
                    bool isNull = input.readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = input.readInt();
                    serializerHolder->getDataSerializer().read(input, object);
                };

                template<typename  T>
                T readObjectResolved(ObjectDataInput& input, void *tag) {
                    bool isNull = input.readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = input.readInt();
                    SerializerBase *serializer = serializerHolder->serializerFor(getSerializerId(object));
                    if (serializer) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);;
                        s->read(input, object);
                        return object;
                    } else {
                        throw exception::IOException("ObjectDataInput::readObjectResolved(ObjectDataInput& input, void *tag)", "No serializer found for serializerId :" + util::to_string(typeId) + ", typename :" + typeid(T).name());
                    }

                };
                  */
                int position();

                void position(int newPos);

            private:
                const std::vector<byte>& buffer;
                int pos;
                SerializerHolder *serializerHolder;
                SerializationContext *serializationContext;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                ObjectDataInput(const ObjectDataInput&);

                ObjectDataInput& operator = (const ObjectDataInput&);

            };
        }
    }
}
#endif /* HAZELCAST_DATA_INPUT */
