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
#include "IOException.h"
#include "Serializer.h"
#include "SerializerHolder.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class SerializationService;

            class SerializationContext;

            class DataInput;

            typedef unsigned char byte;

            class ObjectDataInput {
            public:
                ObjectDataInput(DataInput&, SerializerHolder&, SerializationContext&);//TODO 1 is both constructors are needed

                ObjectDataInput(const std::vector<byte>&, SerializerHolder&, SerializationContext&);

                void setSerializationContext(SerializationContext *context);

                SerializationContext *getSerializationContext();

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

                template<typename  T>
                T readObject() {
                    T *tag;
                    return readObjectResolved<T>(tag);
                };

                template<typename  T>
                T readObjectResolved(Portable *tag) {
                    bool isNull = readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = readInt();

                    ClassDefinition classDefinition;
                    classDefinition.readData(*this);
                    int factoryId = classDefinition.getFactoryId();
                    int classId = classDefinition.getClassId();
                    int version = classDefinition.getVersion();

                    serializerHolder.getPortableSerializer().read(*this, object, factoryId, classId, version);
                };

                template<typename  T>
                T readObjectResolved(DataSerializable *tag) {
                    bool isNull = readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = readInt();
                    serializerHolder.getDataSerializer().read(*this, object);
                };

                template<typename  T>
                T readObjectResolved(void *tag) {
                    bool isNull = readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = readInt();
                    SerializerBase *serializer = serializerHolder.serializerFor(getSerializerId(object));
                    if (serializer) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);;
                        s->read(*this, object);
                        return object;
                    } else {
                        throw exception::IOException("ObjectDataInput::readObjectResolved(ObjectDataInput& input, void *tag)", "No serializer found for serializerId :" + util::to_string(typeId) + ", typename :" + typeid(T).name());
                    }

                };

                int position();

                void position(int newPos);

            private:
                const std::vector<byte>& buffer;
                int pos;
                SerializerHolder& serializerHolder;
                SerializationContext& serializationContext;

                static int const STRING_CHUNK_SIZE = 16 * 1024;

                std::string readShortUTF();

                ObjectDataInput(const ObjectDataInput&);

                ObjectDataInput& operator = (const ObjectDataInput&);

            };
        }
    }
}
#endif /* HAZELCAST_DATA_INPUT */
