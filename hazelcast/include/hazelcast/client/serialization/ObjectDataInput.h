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
#include "hazelcast/client/serialization/pimpl/SerializationContext.h"
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

                class SerializationContext;

                class DataInput;
            }


            class HAZELCAST_API ObjectDataInput {
            public:
                ObjectDataInput(pimpl::DataInput &, pimpl::SerializationContext &);

                pimpl::SerializationContext *getSerializationContext();

                void readFully(std::vector<byte> &);

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
                boost::shared_ptr<T> readObject() {
                    T *tag = NULL;
                    return readObjectResolved<T>(tag);
                };

                template<typename  T>
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
                    serializationContext.registerClassDefinition(classDefinition);
                    serializerHolder.getPortableSerializer().read(dataInput, *object, factoryId, classId, version);
                    return object;
                };

                template<typename  T>
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

                template<typename  T>
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

                int position();

                void position(int newPos);

            private:
                pimpl::DataInput &dataInput;
                pimpl::SerializationContext &serializationContext;
                pimpl::SerializerHolder &serializerHolder;

                ObjectDataInput(const ObjectDataInput &);

                void operator = (const ObjectDataInput &);
            };
        }
    }
}
#endif /* HAZELCAST_DATA_INPUT */
