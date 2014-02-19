//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_SERVICE
#define HAZELCAST_SERIALIZATION_SERVICE

#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/serialization/PortableSerializer.h"
#include "hazelcast/client/serialization/DataSerializer.h"
#include "hazelcast/client/Portable.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/DataInput.h"
#include "hazelcast/client/serialization/SerializerHolder.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/serialization/SerializationConstants.h"
#include <boost/shared_ptr.hpp>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class HAZELCAST_API SerializationService {
            public:

                SerializationService(int version);

                /**
                *
                *  return false if a serializer is already given corresponding to serializerId
                */
                bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                template<typename T>//MTODO we probably do not need template
                Data toData(const Portable *portable) {
                    const T *object = static_cast<const T *>(portable);
                    DataOutput output;
                    getSerializerHolder().getPortableSerializer().write(output, *object);
                    int factoryId = object->getFactoryId();
                    int classId = object->getClassId();
                    Data data;
                    data.setType(serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE);
                    data.cd = serializationContext.lookup(factoryId, classId);
                    data.setBuffer(output.toByteArray());
                    return data;
                };

                template<typename T> //MTODO we probably do not need template
                Data toData(const IdentifiedDataSerializable *dataSerializable) {
                    const T *object = static_cast<const T *>(dataSerializable);
                    Data data;
                    DataOutput dataOutput;
                    ObjectDataOutput output(dataOutput, serializationContext);
                    getSerializerHolder().getDataSerializer().write(output, *object);
                    data.setType(serialization::SerializationConstants::CONSTANT_TYPE_DATA);
                    data.setBuffer(output.toByteArray());
                    return data;
                };

                template<typename T>
                Data toData(const void *serializable) {
                    const T *object = static_cast<const T *>(serializable);
                    Data data;
                    DataOutput dataOutput;
                    ObjectDataOutput output(dataOutput, serializationContext);
                    int type = object->getSerializerId();
                    boost::shared_ptr<SerializerBase> serializer = serializerFor(type);
                    if (serializer.get() != NULL) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                        s->write(output, *object);
                    } else {
                        throw exception::IOException("SerializationService::toData", "No serializer found for serializerId :" + util::to_string(type) + ", typename :" + typeid(T).name());
                    }
                    data.setType(type);
                    data.setBuffer(output.toByteArray());
                    return data;
                };

                template<typename T>
                inline boost::shared_ptr<T> toObject(const Data &data) {
                    T *tag = NULL;
                    return toObjectResolved<T>(data, tag);
                };

                template<typename T>
                inline boost::shared_ptr<T> toObjectResolved(const Data &data, Portable *tag) {
                    if (data.bufferSize() == 0)
                        return boost::shared_ptr<T>();
                    boost::shared_ptr<T> object(new T);
                    DataInput dataInput(*(data.buffer.get()));

                    serializationContext.registerClassDefinition(data.cd);
                    int factoryId = data.cd->getFactoryId();
                    int classId = data.cd->getClassId();
                    int version = data.cd->getVersion();
                    getSerializerHolder().getPortableSerializer().read(dataInput, *object, factoryId, classId, version);
                    return object;
                };

                template<typename T>
                inline boost::shared_ptr<T> toObjectResolved(const Data &data, IdentifiedDataSerializable *tag) {
                    if (data.bufferSize() == 0)
                        return boost::shared_ptr<T>();
                    boost::shared_ptr<T> object(new T);
                    DataInput dataInput(*(data.buffer.get()));
                    ObjectDataInput objectDataInput(dataInput, serializationContext);
                    getSerializerHolder().getDataSerializer().read(objectDataInput, *object);
                    return object;
                };

                template<typename T>
                inline boost::shared_ptr<T> toObjectResolved(const Data &data, void *tag) {
                    if (data.bufferSize() == 0)
                        return boost::shared_ptr<T>();
                    boost::shared_ptr<T> object(new T);
                    DataInput dataInput(*(data.buffer.get()));
                    ObjectDataInput objectDataInput(dataInput, serializationContext);
                    boost::shared_ptr<SerializerBase> serializer = serializerFor(object->getSerializerId());
                    if (serializer.get() != NULL) {
                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                        s->read(objectDataInput, *object);
                        return object;
                    } else {
                        throw exception::IOException("SerializationService::toData", "No serializer found for serializerId :" + util::to_string(data.getType()) + ", typename :" + typeid(T).name());
                    }
                };

                SerializationContext &getSerializationContext();

                SerializerHolder &getSerializerHolder();

            private:
                boost::shared_ptr<SerializerBase> serializerFor(int typeId);

                SerializationService(const SerializationService &);

                SerializationService& operator =(const SerializationService&);

                SerializationContext serializationContext;

            };


            template<>
            inline Data SerializationService::toData<byte >(const void *serializable) {
                DataOutput output;
                const byte *object = static_cast<const byte *>(serializable);
                output.writeByte(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_BYTE);
                return data;
            };


            template<>
            inline Data SerializationService::toData<bool>(const void *serializable) {
                DataOutput output;
                const bool *object = static_cast<const bool *>(serializable);
                output.writeBoolean(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_BOOLEAN);
                return data;
            };


            template<>
            inline Data SerializationService::toData<char>(const void *serializable) {
                DataOutput output;
                const char *object = static_cast<const char *>(serializable);
                output.writeChar(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_CHAR);
                return data;
            };


            template<>
            inline Data SerializationService::toData<short>(const void *serializable) {
                DataOutput output;
                const short *object = static_cast<const short *>(serializable);
                output.writeShort(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_SHORT);
                return data;
            };


            template<>
            inline Data SerializationService::toData<int>(const void *serializable) {
                DataOutput output;
                const int *object = static_cast<const int *>(serializable);
                output.writeInt(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_INTEGER);
                return data;
            };


            template<>
            inline Data SerializationService::toData<long>(const void *serializable) {
                DataOutput output;
                const long *object = static_cast<const long *>(serializable);
                output.writeLong(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_LONG);
                return data;
            };


            template<>
            inline Data SerializationService::toData<float>(const void *serializable) {
                DataOutput output;
                const float *object = static_cast<const float *>(serializable);
                output.writeFloat(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_FLOAT);
                return data;
            };


            template<>
            inline Data SerializationService::toData<double>(const void *serializable) {
                DataOutput output;
                const double *object = static_cast<const double *>(serializable);
                output.writeDouble(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_DOUBLE);
                return data;
            };

            template<>
            inline Data SerializationService::toData<std::vector<char> >(const void *serializable) {
                DataOutput output;
                const std::vector<char> *object = static_cast<const std::vector<char> *>(serializable);
                output.writeCharArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::vector<short> >(const void *serializable) {
                DataOutput output;
                const std::vector<short> *object = static_cast<const std::vector<short> *>(serializable);
                output.writeShortArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::vector<int> >(const void *serializable) {
                DataOutput output;
                const std::vector<int> *object = static_cast<const std::vector<int> *>(serializable);
                output.writeIntArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::vector<long> >(const void *serializable) {
                DataOutput output;
                const std::vector<long> *object = static_cast<const std::vector<long> *>(serializable);
                output.writeLongArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::vector<float> >(const void *serializable) {
                DataOutput output;
                const std::vector<float> *object = static_cast<const std::vector<float> *>(serializable);
                output.writeFloatArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::vector<double> >(const void *serializable) {
                DataOutput output;
                const std::vector<double> *object = static_cast<const std::vector<double> *>(serializable);
                output.writeDoubleArray(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY);
                return data;
            };


            template<>
            inline Data SerializationService::toData<std::string>(const void *serializable) {
                DataOutput output;
                const std::string *object = static_cast<const std::string *>(serializable);
                output.writeUTF(*object);
                Data data;
                data.setBuffer(output.toByteArray());
                data.setType(SerializationConstants::CONSTANT_TYPE_STRING);
                return data;
            };

            template<>
            inline boost::shared_ptr<byte> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<byte>();
                boost::shared_ptr<byte> object(new byte);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readByte();
                return object;
            };

            template<>
            inline boost::shared_ptr<bool> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<bool>();
                boost::shared_ptr<bool> object(new bool);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readBoolean();
                return object;
            };

            template<>
            inline boost::shared_ptr<char> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<char>();
                boost::shared_ptr<char> object(new char);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readChar();
                return object;
            };

            template<>
            inline boost::shared_ptr<short> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<short>();
                boost::shared_ptr<short> object(new short);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readShort();
                return object;
            };

            template<>
            inline boost::shared_ptr<int> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<int>();
                boost::shared_ptr<int> object(new int);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readInt();
                return object;
            };

            template<>
            inline boost::shared_ptr<long> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<long>();
                boost::shared_ptr<long> object(new long);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readLong();
                return object;
            };

            template<>
            inline boost::shared_ptr<float> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<float>();
                boost::shared_ptr<float> object(new float);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readFloat();
                return object;
            };

            template<>
            inline boost::shared_ptr<double> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<double>();
                boost::shared_ptr<double> object(new double);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readDouble();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::vector<char> > SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<char> >();
                boost::shared_ptr<std::vector<char> > object(new std::vector<char>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readCharArray();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::vector<short> >  SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<short> >();
                boost::shared_ptr<std::vector<short> > object(new std::vector<short>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readShortArray();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::vector<int> > SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<int> >();
                boost::shared_ptr<std::vector<int> > object(new std::vector<int>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readIntArray();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::vector<long> > SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<long> >();
                boost::shared_ptr<std::vector<long> > object(new std::vector<long>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readLongArray();
                return object;
            };

            template<>
            inline boost::shared_ptr< std::vector<float> >  SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<float> >();
                boost::shared_ptr<std::vector<float> > object(new std::vector<float>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readFloatArray();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::vector<double> > SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::vector<double> >();
                boost::shared_ptr<std::vector<double> > object(new std::vector<double>);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readDoubleArray();
                return object;
            };

            template<>
            inline boost::shared_ptr<std::string> SerializationService::toObject(const Data &data) {
                
                if (data.bufferSize() == 0) return boost::shared_ptr<std::string >();
                boost::shared_ptr<std::string > object(new std::string);
                DataInput dataInput(*(data.buffer.get()));
                *object = dataInput.readUTF();
                return object;
            };
        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
