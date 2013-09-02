//
//  ObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "IOException.h"
#include "Serializer.h"
#include "SerializerHolder.h"
#include "ClassDefinition.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        class Portable;

        class IdentifiedDataSerializable;

        namespace serialization {

            class SerializationService;

            class SerializationContext;

            class DataInput;

            typedef unsigned char byte;

            class ObjectDataInput {
            public:
                ObjectDataInput(DataInput&, SerializationContext&);

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
                    classDefinition.readData(dataInput);
                    int factoryId = classDefinition.getFactoryId();
                    int classId = classDefinition.getClassId();
                    int version = classDefinition.getVersion();

                    serializerHolder.getPortableSerializer().read(dataInput, object, factoryId, classId, version);
                    return object;
                };

                template<typename  T>
                T readObjectResolved(IdentifiedDataSerializable *tag) {
                    bool isNull = readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = readInt();
                    serializerHolder.getDataSerializer().read(*this, object);
                    return object;
                };

                template<typename  T>
                T readObjectResolved(void *tag) {
                    bool isNull = readBoolean();
                    T object;
                    if (isNull) {
                        return object;
                    }
                    const int typeId = readInt();
                    SerializerBase *serializer = serializerHolder.serializerFor(object.getSerializerId());
                    if (serializer) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->read(*this, object);
                        return object;
                    } else {
                        throw exception::IOException("ObjectDataInput::readObjectResolved(ObjectDataInput& input, void *tag)", "No serializer found for serializerId :" + util::to_string(typeId) + ", typename :" + typeid(T).name());
                    }

                };

                int position();

                void position(int newPos);

            private:
                DataInput& dataInput;
                SerializationContext& serializationContext;
                SerializerHolder& serializerHolder;

                ObjectDataInput(const ObjectDataInput&);

                void operator = (const ObjectDataInput&);
            };
        }
    }
}
#endif /* HAZELCAST_DATA_INPUT */
