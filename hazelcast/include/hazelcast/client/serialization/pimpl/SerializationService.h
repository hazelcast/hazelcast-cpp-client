//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_SERVICE
#define HAZELCAST_SERIALIZATION_SERVICE

#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ByteBuffer.h"
#include <boost/shared_ptr.hpp>
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class SerializationConfig;

        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API SerializationService {
                public:

                    SerializationService(const SerializationConfig& serializationConfig);

                    /**
                    *
                    *  return false if a serializer is already given corresponding to serializerId
                    */
                    bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                    template<typename T>
                    Data toData(const Portable *portable) {
                        DataOutput output;

                        // write type
                        output.writeInt(SerializationConstants::CONSTANT_TYPE_PORTABLE);

                        // write false (i.e. 0) for hasPartitionHash, since this has no use
                        // in terms of c++ client impl.
                        output.writeBoolean(false);

                        const T *object = static_cast<const T *>(portable);
                        getSerializerHolder().getPortableSerializer().write(output, *object);

                        Data data(output.toByteArray());
                        return data;
                    };

                    template<typename T>
                    Data toData(const IdentifiedDataSerializable *dataSerializable) {
                        DataOutput output;

                        // write type
                        output.writeInt(SerializationConstants::CONSTANT_TYPE_DATA);

                        // write false (i.e. 0) for hasPartitionHash, since this has no use
                        // in terms of c++ client impl.
                        output.writeBoolean(false);

                        const T *object = static_cast<const T *>(dataSerializable);
                        ObjectDataOutput dataOutput(output, portableContext);
                        getSerializerHolder().getDataSerializer().write(dataOutput, *object);

                        Data data(output.toByteArray());
                        return data;
                    };

                    template<typename T>
                    Data toData(const void *serializable) {
                        DataOutput output;

                        const T *object = static_cast<const T *>(serializable);

                        // write type
                        int type = object->getTypeId();
                        output.writeInt(type);

                        // write false (i.e. 0) for hasPartitionHash, since this has no use
                        // in terms of c++ client impl.
                        output.writeBoolean(false);

                        ObjectDataOutput dataOutput(output, portableContext);

                        boost::shared_ptr<SerializerBase> serializer = serializerFor(type);
                        if (serializer.get() != NULL) {
                            Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                            s->write(dataOutput, *object);
                        } else {
                            const std::string &message = "No serializer found for serializerId :"
                                    + ::hazelcast::util::IOUtil::to_string(type) + ", typename :" + typeid(T).name();
                            throw exception::HazelcastSerializationException("SerializationService::toData", message);
                        }

                        Data data(output.toByteArray());
                        return data;
                    };

                    template<typename T>
                    inline boost::shared_ptr<T> toObject(const Data &data) {
                        T *tag = NULL;
                        return toObjectResolved<T>(data, tag);
                    };

                    PortableContext &getPortableContext();

                private:
                    template<typename T>
                    inline boost::shared_ptr<T> toObjectResolved(const Data &data, Portable *tag) {
                        if (isNullData(data)) {
                            return boost::shared_ptr<T>();
                        }

                        DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                        int typeId = data.getType();

                        checkClassType(SerializationConstants::CONSTANT_TYPE_PORTABLE, typeId);

                        boost::shared_ptr<T> object(new T);
                        getSerializerHolder().getPortableSerializer().read(dataInput, *object);

                        return object;
                    };

                    template<typename T>
                    inline boost::shared_ptr<T> toObjectResolved(const Data &data, IdentifiedDataSerializable *tag) {
                        if (isNullData(data)) {
                            return boost::shared_ptr<T>();
                        }

                        DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                        int typeId = data.getType();

                        checkClassType(SerializationConstants::CONSTANT_TYPE_DATA, typeId);

                        boost::shared_ptr<T> object(new T);
                        ObjectDataInput objectDataInput(dataInput, portableContext);
                        getSerializerHolder().getDataSerializer().read(objectDataInput, *object);

                        return object;
                    };

                    template<typename T>
                    inline boost::shared_ptr<T> toObjectResolved(const Data &data, void *tag) {
                        if (isNullData(data)) {
                            return boost::shared_ptr<T>();
                        }

                        DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                        boost::shared_ptr<T> object(new T);
                        int typeId = object->getTypeId();

                        ObjectDataInput objectDataInput(dataInput, portableContext);
                        boost::shared_ptr<SerializerBase> serializer = serializerFor(typeId);
                        if (serializer.get() != NULL) {
                            Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                            s->read(objectDataInput, *object);
                        } else {
                            const std::string &message = "No serializer found for serializerId :"
                                    + ::hazelcast::util::IOUtil::to_string(object->getTypeId()) + ", typename :" + typeid(T).name();
                            throw exception::HazelcastSerializationException("SerializationService::toObject", message);
                        }

                        return object;
                    };

                    SerializerHolder &getSerializerHolder();

                    boost::shared_ptr<SerializerBase> serializerFor(int typeId);

                    SerializationService(const SerializationService &);

                    SerializationService &operator = (const SerializationService &);

                    PortableContext portableContext;

                    const SerializationConfig& serializationConfig;

                    void checkClassType(int expectedType, int currentType);

                    inline static bool isNullData(const Data &data) {
                        return data.dataSize() == 0 || data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                };


                template<>
                inline Data SerializationService::toData<byte >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BYTE);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const byte *object = static_cast<const byte *>(serializable);
                    output.writeByte(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<bool>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BOOLEAN);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const bool *object = static_cast<const bool *>(serializable);
                    output.writeBoolean(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<char>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_CHAR);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const char *object = static_cast<const char *>(serializable);
                    output.writeChar(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<short>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_SHORT);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const short *object = static_cast<const short *>(serializable);
                    output.writeShort(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<int>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_INTEGER);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const int *object = static_cast<const int *>(serializable);
                    output.writeInt(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<long>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_LONG);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const long *object = static_cast<const long *>(serializable);
                    output.writeLong(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<float>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_FLOAT);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const float *object = static_cast<const float *>(serializable);
                    output.writeFloat(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<double>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_DOUBLE);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const double *object = static_cast<const double *>(serializable);
                    output.writeDouble(*object);

                    Data data(output.toByteArray());
                    return data;
                };

                template<>
                inline Data SerializationService::toData<std::vector<char> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<char> *object = static_cast<const std::vector<char> *>(serializable);
                    output.writeCharArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::vector<short> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<short> *object = static_cast<const std::vector<short> *>(serializable);
                    output.writeShortArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::vector<int> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<int> *object = static_cast<const std::vector<int> *>(serializable);
                    output.writeIntArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::vector<long> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<long> *object = static_cast<const std::vector<long> *>(serializable);
                    output.writeLongArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::vector<float> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<float> *object = static_cast<const std::vector<float> *>(serializable);
                    output.writeFloatArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::vector<double> >(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::vector<double> *object = static_cast<const std::vector<double> *>(serializable);
                    output.writeDoubleArray(*object);

                    Data data(output.toByteArray());
                    return data;
                };


                template<>
                inline Data SerializationService::toData<std::string>(const void *serializable) {
                    DataOutput output;

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_STRING);

                    // write false (i.e. 0) for hasPartitionHash, since this has no use
                    // in terms of c++ client impl.
                    output.writeBoolean(false);

                    const std::string *object = static_cast<const std::string *>(serializable);
                    output.writeUTF(*object);

                    Data data(output.toByteArray());
                    return data;
                };

                template<>
                inline boost::shared_ptr<byte> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<byte>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_BYTE, typeId);

                    boost::shared_ptr<byte> object(new byte);

                    *object = dataInput.readByte();

                    return object;
                };

                template<>
                inline boost::shared_ptr<bool> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<bool>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_BOOLEAN, typeId);

                    boost::shared_ptr<bool> object(new bool);

                    *object = dataInput.readBoolean();

                    return object;
                };

                template<>
                inline boost::shared_ptr<char> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<char>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_CHAR, typeId);

                    boost::shared_ptr<char> object(new char);

                    *object = dataInput.readChar();

                    return object;
                };

                template<>
                inline boost::shared_ptr<short> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<short>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_SHORT, typeId);

                    boost::shared_ptr<short> object(new short);

                    *object = dataInput.readShort();

                    return object;
                };

                template<>
                inline boost::shared_ptr<int> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<int>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_INTEGER, typeId);

                    boost::shared_ptr<int> object(new int);

                    *object = dataInput.readInt();

                    return object;
                };

                template<>
                inline boost::shared_ptr<long> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<long>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_LONG, typeId);

                    boost::shared_ptr<long> object(new long);

                    *object = (long)dataInput.readLong();

                    return object;
                };

                template<>
                inline boost::shared_ptr<float> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<float>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_FLOAT, typeId);

                    boost::shared_ptr<float> object(new float);

                    *object = dataInput.readFloat();

                    return object;
                };

                template<>
                inline boost::shared_ptr<double> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<double>();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_DOUBLE, typeId);

                    boost::shared_ptr<double> object(new double);

                    *object = dataInput.readDouble();

                    return object;
                };

                template<>
                inline boost::shared_ptr<std::vector<char> > SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<char> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY, typeId);

                    return dataInput.readCharArrayAsPtr();
                };

                template<>
                inline boost::shared_ptr<std::vector<short> >  SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<short> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY, typeId);

                    boost::shared_ptr<std::vector<short> > object(new std::vector<short>);

                    *object = dataInput.readShortArray();

                    return object;
                };

                template<>
                inline boost::shared_ptr<std::vector<int> > SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<int> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY, typeId);

                    boost::shared_ptr<std::vector<int> > object(new std::vector<int>);

                    *object = dataInput.readIntArray();

                    return object;
                };

                template<>
                inline boost::shared_ptr<std::vector<long> > SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<long> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY, typeId);

                    boost::shared_ptr<std::vector<long> > object(new std::vector<long>);

                    *object = dataInput.readLongArray();

                    return object;
                };

                template<>
                inline boost::shared_ptr< std::vector<float> >  SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<float> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY, typeId);

                    boost::shared_ptr<std::vector<float> > object(new std::vector<float>);

                    *object = dataInput.readFloatArray();

                    return object;
                };

                template<>
                inline boost::shared_ptr<std::vector<double> > SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::vector<double> >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY, typeId);

                    boost::shared_ptr<std::vector<double> > object(new std::vector<double>);

                    *object = dataInput.readDoubleArray();

                    return object;
                };

                template<>
                inline boost::shared_ptr<std::string> SerializationService::toObject(const Data &data) {
                    if (isNullData(data)) {
                        return boost::shared_ptr<std::string >();
                    }

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    checkClassType(SerializationConstants::CONSTANT_TYPE_STRING, typeId);

                    boost::shared_ptr<std::string > object(new std::string);

                    *object = dataInput.readUTF();

                    return object;
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_SERVICE */

