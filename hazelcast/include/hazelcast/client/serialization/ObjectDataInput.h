//
//  ObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/util/IOUtil.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

namespace hazelcast {
    namespace client {
        class Portable;

        class IdentifiedDataSerializable;

        namespace serialization {

            namespace pimpl {
                class SerializationService;

                class PortableContext;

                class DataInput;
            }

            /**
            * Provides deserialization methods for primitives types, arrays of primitive types
            * Portable, IdentifiedDataSerializable and custom serializable types
            */
            class HAZELCAST_API ObjectDataInput {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(pimpl::DataInput &, pimpl::PortableContext &);

                /**
                * Internal API.
                * @return portableContext
                */
                pimpl::PortableContext *getPortableContext();

                /**
                * fills all content to given byteArray
                * @param byteArray to fill the data in
                */
                void readFully(std::vector<byte> &byteArray);

                /**
                *
                * @param i number of bytes to skip
                */
                int skipBytes(int i);

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                bool readBoolean();

                /**
                * @return the byte read
                * @throws IOException if it reaches end of file before finish reading
                */
                byte readByte();

                /**
                * @return the short read
                * @throws IOException if it reaches end of file before finish reading
                */
                short readShort();

                /**
                * @return the char read
                * @throws IOException if it reaches end of file before finish reading
                */
                char readChar();

                /**
                * @return the int read
                * @throws IOException if it reaches end of file before finish reading
                */
                int readInt();

                /**
                * @return the long read
                * @throws IOException if it reaches end of file before finish reading
                */
                long readLong();

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                float readFloat();

                /**
                * @return the double read
                * @throws IOException if it reaches end of file before finish reading
                */
                double readDouble();

                /**
                * @return the utf string read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::string readUTF();

                /**
                * @return the byte array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<byte> readByteArray();

                /**
                * @return the char array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<char> readCharArray();

                /**
                * @return the int array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<int> readIntArray();

                /**
                * @return the long array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<long> readLongArray();

                /**
                * @return the double array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<double> readDoubleArray();

                /**
                * @return the float array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<float> readFloatArray();

                /**
                * @return the short array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::vector<short> readShortArray();

                /**
                * Object can be Portable, IdentifiedDataSerializable or custom serializable
                * for custom serialization @see Serializer
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                boost::shared_ptr<T> readObject() {
                    T *tag = NULL;
                    return readObjectResolved<T>(tag);
                };

                /**
                * @return current position index
                */
                int position();

                /**
                * Move cursor to given index
                * @param newPos new position index to be set
                */
                void position(int newPos);

            private:

                template<typename T>
                boost::shared_ptr<T> readObjectResolved(Portable *tag) {
                    bool isNull = readBoolean();
                    boost::shared_ptr<T> object;
                    if (isNull) {
                        return object;
                    }
                    object.reset(new T);

                    readInt();

                    boost::shared_ptr<pimpl::ClassDefinition> classDefinition(new pimpl::ClassDefinition());
                    classDefinition->readData(dataInput);
                    int factoryId = classDefinition->getFactoryId();
                    int classId = classDefinition->getClassId();
                    int version = classDefinition->getVersion();
                    portableContext.registerClassDefinition(classDefinition);
                    serializerHolder.getPortableSerializer().read(dataInput, *object, factoryId, classId, version);
                    return object;
                };

                template<typename T>
                boost::shared_ptr<T> readObjectResolved(IdentifiedDataSerializable *tag) {
                    bool isNull = readBoolean();
                    boost::shared_ptr<T> object;
                    if (isNull) {
                        return object;
                    }
                    object.reset(new T);
                    readInt();
                    serializerHolder.getDataSerializer().read(*this, *object);
                    return object;
                };

                template<typename T>
                boost::shared_ptr<T> readObjectResolved(void *tag) {
                    bool isNull = readBoolean();
                    boost::shared_ptr<T> object;
                    if (isNull) {
                        return object;
                    }
                    object.reset(new T);
                    const int typeId = readInt();
                    boost::shared_ptr<SerializerBase> serializer = serializerHolder.serializerFor(object.getSerializerId());
                    if (serializer.get() != NULL) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer);
                        s->read(*this, *object);
                        return object;
                    } else {
                        const std::string &message = "No serializer found for serializerId :"
                                + util::IOUtil::to_string(typeId) + ", typename :" + typeid(T).name();
                        throw exception::IOException("ObjectDataInput::readObjectResolved(ObjectDataInput&,void *)", message);
                    }

                };

                pimpl::DataInput &dataInput;
                pimpl::PortableContext &portableContext;
                pimpl::SerializerHolder &serializerHolder;

                ObjectDataInput(const ObjectDataInput &);

                void operator=(const ObjectDataInput &);
            };
        }
    }
}
#endif /* HAZELCAST_DATA_INPUT */

