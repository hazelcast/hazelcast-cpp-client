//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ObjectDataOutput
#define HAZELCAST_ObjectDataOutput

#include "IOException.h"
#include "SerializerHolder.h"
#include "Serializer.h"
#include "Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class DataOutput;

            class ObjectDataOutput {
            public:
                ObjectDataOutput(DataOutput& dataOutput, SerializationContext& serializationContext);

                ObjectDataOutput();

                std::auto_ptr< std::vector<byte> > toByteArray();

                void write(const std::vector<byte>& bytes);

                void writeBoolean(bool b);

                void writeByte(int i);

                void writeShort(int i);

                void writeChar(int i);

                void writeInt(int i);

                void writeLong(long l);

                void writeFloat(float v);

                void writeDouble(double v);

                void writeUTF(const std::string& s);

                void writeByteArray(const std::vector<byte>&  data);

                void writeCharArray(const std::vector<char>& bytes);

                void writeShortArray(const std::vector<short >&  data);

                void writeIntArray(const std::vector<int>&  data);

                void writeLongArray(const std::vector<long >&  data);

                void writeFloatArray(const std::vector<float >&  data);

                void writeDoubleArray(const std::vector<double >&  data);

                void writeByte(int index, int i);

                void writeInt(int index, int v);

                void writeNullObject();

                void writeObject(const Portable *portable);

                void writeObject(const IdentifiedDataSerializable *dataSerializable);

                template<typename T>
                void writeObject(const T *object) {
                    if (isEmpty) return;
                    int type = object->getSerializerId();
                    writeBoolean(true);
                    writeInt(type);
                    SerializerBase *serializer = serializerHolder->serializerFor(type);
                    if (serializer) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->write(*this, *object);
                    } else {
                        throw exception::IOException("ObjectDataOutput::writeObject", "No serializer found for serializerId :" + util::to_string(type) + ", typename :" + typeid(T).name());
                    }
                };

            private:
                DataOutput* dataOutput;
                SerializationContext *context;
                SerializerHolder *serializerHolder;
                bool isEmpty;

                int position();

                void position(int newPos);

                void reset();

                ObjectDataOutput(const ObjectDataOutput&);

                void operator = (const ObjectDataOutput&);
            };

        }
    }
}

#endif //HAZELCAST_ObjectDataOutput
