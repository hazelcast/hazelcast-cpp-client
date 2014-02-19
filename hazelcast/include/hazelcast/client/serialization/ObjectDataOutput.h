//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ObjectDataOutput
#define HAZELCAST_ObjectDataOutput

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/SerializerHolder.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {


            class DataOutput;

            class HAZELCAST_API ObjectDataOutput {
            public:
                ObjectDataOutput(DataOutput &dataOutput, SerializationContext &serializationContext);

                ObjectDataOutput();

                std::auto_ptr< std::vector<byte> > toByteArray();

                void write(const std::vector<byte> &bytes);

                void writeBoolean(bool b);

                void writeByte(int i);

                void writeShort(int i);

                void writeChar(int i);

                void writeInt(int i);

                void writeLong(long l);

                void writeFloat(float v);

                void writeDouble(double v);

                void writeUTF(const std::string &s);

                void writeByteArray(const std::vector<byte> &data);

                void writeCharArray(const std::vector<char> &bytes);

                void writeShortArray(const std::vector<short > &data);

                void writeIntArray(const std::vector<int> &data);

                void writeLongArray(const std::vector<long > &data);

                void writeFloatArray(const std::vector<float > &data);

                void writeDoubleArray(const std::vector<double > &data);

                void writeByte(int index, int i);

                void writeInt(int index, int v);

                void writeNullObject();

                template<typename T>
                void writeObject(const Portable *portable) {
                    if (isEmpty) return;
                    const T *object = static_cast<const T *>(portable);
                    writePortable(portable);
                };

                template<typename T>
                void writeObject(const IdentifiedDataSerializable *dataSerializable) {
                    if (isEmpty) return;
                    const T *object = static_cast<const T *>(dataSerializable);
                    writeIdentifiedDataSerializable(dataSerializable);
                };

                template<typename T>
                void writeObject(const void *serializable) {
                    if (isEmpty) return;
                    const T *object = static_cast<const T *>(serializable);
                    int type = object->getSerializerId();
                    writeBoolean(true);
                    writeInt(type);
                    boost::shared_ptr<SerializerBase> serializer = serializerHolder->serializerFor(type);
                    if (serializer.get() != NULL) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->write(*this, *object);
                    } else {
                        throw exception::IOException("ObjectDataOutput::writeObject", "No serializer found for serializerId :" + util::to_string(type) + ", typename :" + typeid(T).name());
                    }
                };

            private:
                DataOutput *dataOutput;
                SerializationContext *context;
                SerializerHolder *serializerHolder;
                bool isEmpty;

                int position();

                void position(int newPos);

                void writePortable(const Portable *portable);

                void writeIdentifiedDataSerializable(const IdentifiedDataSerializable *dataSerializable);

                ObjectDataOutput(const ObjectDataOutput &);

                void operator = (const ObjectDataOutput &);
            };

        }
    }
}

#endif //HAZELCAST_ObjectDataOutput
