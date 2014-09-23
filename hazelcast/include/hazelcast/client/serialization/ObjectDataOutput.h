//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ObjectDataOutput
#define HAZELCAST_ObjectDataOutput

#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataOutput;
            }

            /**
            * Provides serialization methods for primitive types,a arrays of primitive types, Portable,
            * IdentifiedDataSerializable and custom serializables.
            * For custom serialization @see Serializer
            */
            class HAZELCAST_API ObjectDataOutput {
            public:
                /**
                * Internal API Constructor
                */
                ObjectDataOutput(pimpl::DataOutput &dataOutput, pimpl::PortableContext &portableContext);

                /**
                * Internal API Constructor
                */
                ObjectDataOutput();

                /**
                * @return copy of internal byte array
                */
                std::auto_ptr<std::vector<byte> > toByteArray();

                /**
                * Writes all the bytes in array to stream
                * @param bytes to be written
                */
                void write(const std::vector<byte> &bytes);

                /**
                * @param value the bool value to be written
                */
                void writeBoolean(bool value);

                /**
                * @param value the byte value to be written
                */
                void writeByte(int value);

                /**
                * @param value the short value to be written
                */
                void writeShort(int value);

                /**
                * @param value the char value to be written
                */
                void writeChar(int value);

                /**
                * @param value the int value to be written
                */
                void writeInt(int value);

                /**
                * @param value the long value to be written
                */
                void writeLong(long value);

                /**
                * @param value the float value to be written
                */
                void writeFloat(float value);

                /**
                * @param value the double value to be written
                */
                void writeDouble(double value);

                /**
                * @param value the utf string value to be written
                */
                void writeUTF(const std::string &value);

                /**
                * @param value the utf string value to be written
                */
                void writeByteArray(const std::vector<byte> &value);

                /**
                * @param value the utf string value to be written
                */
                void writeCharArray(const std::vector<char> &value);

                /**
                * @param value the short array value to be written
                */
                void writeShortArray(const std::vector<short> &value);

                /**
                * @param value the int array value to be written
                */
                void writeIntArray(const std::vector<int> &value);

                /**
                * @param value the short array value to be written
                */
                void writeLongArray(const std::vector<long> &value);

                /**
                * @param value the float array value to be written
                */
                void writeFloatArray(const std::vector<float> &value);

                /**
                * @param value the double array value to be written
                */
                void writeDoubleArray(const std::vector<double> &value);

                /**
                * @param object Portable object to be written
                * @see Portable
                * @throws IOException
                */
                template<typename T>
                void writeObject(const Portable *object) {
                    if (isEmpty) return;
                    if(object == NULL){
                        writeBoolean(true);
                        return;
                    }
                    const T *portable = static_cast<const T *>(object);
                    writePortable(object);
                };

                /**
                * @param object IdentifiedDataSerializable object to be written
                * @see IdentifiedDataSerializable
                * @throws IOException
                */
                template<typename T>
                void writeObject(const IdentifiedDataSerializable *object) {
                    if (isEmpty) return;
                    if(object == NULL){
                        writeBoolean(true);
                        return;
                    }
                    const T *dataSerializable = static_cast<const T *>(object);
                    writeIdentifiedDataSerializable(object);
                };

                /**
                * @param object custom serializable object to be written
                * @see Serializer
                * @throws IOException
                */
                template<typename T>
                void writeObject(const void *object) {
                    if (isEmpty) return;
                    if(object == NULL){
                        writeBoolean(true);
                        return;
                    }
                    const T *serializable = static_cast<const T *>(object);
                    int type = serializable->getSerializerId();
                    writeBoolean(false);
                    writeInt(type);
                    boost::shared_ptr<SerializerBase> serializer = serializerHolder->serializerFor(type);
                    if (serializer.get() != NULL) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->write(*this, *serializable);
                    } else {
                        const std::string &message = "No serializer found for serializerId :"
                                + util::IOUtil::to_string(type)
                                + ", typename :" + typeid(T).name();
                        throw exception::HazelcastSerializationException("ObjectDataOutput::writeObject", message);
                    }
                };

            private:
                pimpl::DataOutput *dataOutput;
                pimpl::PortableContext *context;
                pimpl::SerializerHolder *serializerHolder;
                bool isEmpty;

                int position();

                void position(int newPos);

                void writePortable(const Portable *portable);

                void writeIdentifiedDataSerializable(const IdentifiedDataSerializable *dataSerializable);

                ObjectDataOutput(const ObjectDataOutput &);

                void operator=(const ObjectDataOutput &);
            };

        }
    }
}

#endif //HAZELCAST_ObjectDataOutput

