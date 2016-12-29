/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/util/Util.h"
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IClassCastException.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                #define CHECK_NULL(type) \
                if (isNullData(data)) { \
                    return std::auto_ptr<type >(); \
                }

                SerializationService::SerializationService(const SerializationConfig& serializationConfig)
                : portableContext(serializationConfig.getPortableVersion(), constants)
                , serializationConfig(serializationConfig) {
                    std::vector<boost::shared_ptr<SerializerBase> > const& serializers = serializationConfig.getSerializers();
                    std::vector<boost::shared_ptr<SerializerBase> >::const_iterator it;
                    SerializerHolder& serializerHolder = getSerializerHolder();
                    for (it = serializers.begin(); it < serializers.end(); ++it) {
                        serializerHolder.registerSerializer(*it);
                    }
                }

                SerializerHolder& SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }


                bool SerializationService::registerSerializer(boost::shared_ptr<SerializerBase> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void SerializationService::writeHash(DataOutput &out) {
                    // TODO: Implement PartitionStrategy and write calculated hash.
                    out.writeInt(0);
                }

                template<>
                Data SerializationService::toData<byte>(const byte  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BYTE);

                    output.writeByte(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<bool>(const bool  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BOOLEAN);

                    output.writeBoolean(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<char>(const char  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_CHAR);

                    output.writeChar(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<short>(const short  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_SHORT);

                    output.writeShort(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<int>(const int  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_INTEGER);

                    output.writeInt(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<long>(const long *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_LONG);

                    output.writeLong(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<float>(const float  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_FLOAT);

                    output.writeFloat(*object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<double>(const double  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_DOUBLE);

                    output.writeDouble(*object);

                    Data data(output.toByteArray());
                    return data;
                }

                template<>
                Data SerializationService::toData<std::vector<char> >(const std::vector<char> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY);

                    output.writeCharArray(object);

                    Data data(output.toByteArray());
                    return data;
                }

                template<>
                Data SerializationService::toData<std::vector<bool> >(const std::vector<bool> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY);

                    output.writeBooleanArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::vector<short> >(const std::vector<short> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY);

                    output.writeShortArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::vector<int> >(const std::vector<int> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY);

                    output.writeIntArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::vector<long> >(const std::vector<long> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY);

                    output.writeLongArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::vector<float> >(const std::vector<float> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY);

                    output.writeFloatArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::vector<double> >(const std::vector<double> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY);

                    output.writeDoubleArray(object);

                    Data data(output.toByteArray());
                    return data;
                }


                template<>
                Data SerializationService::toData<std::string>(const std::string  *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_STRING);

                    output.writeUTF(object);

                    Data data(output.toByteArray());
                    return data;
                }

                template<>
                Data SerializationService::toData<std::vector<std::string> >(const std::vector<std::string> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_STRING_ARRAY);

                    output.writeUTFArray(object);

                    Data data(output.toByteArray());
                    return data;
                }

                template<>
                std::auto_ptr<byte> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(byte);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_BYTE, typeId);

                    std::auto_ptr<byte> object(new byte);

                    *object = dataInput.readByte();

                    return object;
                }

                template<>
                std::auto_ptr<bool> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(bool);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_BOOLEAN, typeId);

                    std::auto_ptr<bool> object(new bool);

                    *object = dataInput.readBoolean();

                    return object;
                }

                template<>
                std::auto_ptr<char> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(char);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_CHAR, typeId);

                    std::auto_ptr<char> object(new char);

                    *object = dataInput.readChar();

                    return object;
                }

                template<>
                std::auto_ptr<short> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(short);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_SHORT, typeId);

                    std::auto_ptr<short> object(new short);

                    *object = dataInput.readShort();

                    return object;
                }

                template<>
                std::auto_ptr<int> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(int);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_INTEGER, typeId);

                    std::auto_ptr<int> object(new int);

                    *object = dataInput.readInt();

                    return object;
                }

                template<>
                std::auto_ptr<long> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(long);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_LONG, typeId);

                    std::auto_ptr<long> object(new long);

                    *object = (long)dataInput.readLong();

                    return object;
                }

                template<>
                std::auto_ptr<float> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(float);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_FLOAT, typeId);

                    std::auto_ptr<float> object(new float);

                    *object = dataInput.readFloat();

                    return object;
                }

                template<>
                std::auto_ptr<double> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(double);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_DOUBLE, typeId);

                    std::auto_ptr<double> object(new double);

                    *object = dataInput.readDouble();

                    return object;
                }

                template<>
                std::auto_ptr<std::vector<char> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<char>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY, typeId);

                    return std::auto_ptr<std::vector<char> > (dataInput.readCharArray());
                }

                template<>
                std::auto_ptr<std::vector<bool> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<bool>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY, typeId);

                    return std::auto_ptr<std::vector<bool> > (dataInput.readBooleanArray());
                }

                template<>
                std::auto_ptr<std::vector<short> >  SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<short>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY, typeId);

                    return std::auto_ptr<std::vector<short> > (dataInput.readShortArray());
                }

                template<>
                std::auto_ptr<std::vector<int> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<int>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY, typeId);

                    return std::auto_ptr<std::vector<int> > (dataInput.readIntArray());
                }

                template<>
                std::auto_ptr<std::vector<long> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<long>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY, typeId);

                    return std::auto_ptr<std::vector<long> > (dataInput.readLongArray());
                }

                template<>
                std::auto_ptr< std::vector<float> >  SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<float>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY, typeId);

                    return std::auto_ptr<std::vector<float> > (dataInput.readFloatArray());
                }

                template<>
                std::auto_ptr<std::vector<double> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<double>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY, typeId);

                    return std::auto_ptr<std::vector<double> > (dataInput.readDoubleArray());
                }

                template<>
                std::auto_ptr<std::string> SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::string);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_STRING, typeId);

                    return std::auto_ptr<std::string> (dataInput.readUTF());
                }

                template<>
                std::auto_ptr<std::vector<std::string> > SerializationService::toObject(const Data &data) {
                    CHECK_NULL(std::vector<std::string>);

                    DataInput dataInput(data.toByteArray(), Data::DATA_OFFSET);

                    int typeId = data.getType();

                    constants.checkClassType(SerializationConstants::CONSTANT_TYPE_STRING_ARRAY, typeId);

                    return std::auto_ptr<std::vector<std::string> > (dataInput.readUTFArray());
                }

                template<>
                std::auto_ptr<Data> SerializationService::toObject(const Data *data) {
                    return std::auto_ptr<Data>((Data *) data);
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }
            }
        }
    }
}
