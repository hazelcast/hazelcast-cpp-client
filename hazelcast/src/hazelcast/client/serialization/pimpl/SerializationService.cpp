/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig),
                          serializationConfig(serializationConfig) {
                    registerConstantSerializers();

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
                Data SerializationService::toData<int16_t>(const int16_t  *object) {
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
                Data SerializationService::toData<int32_t>(const int32_t  *object) {
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
                Data SerializationService::toData<int64_t>(const int64_t *object) {
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
                Data SerializationService::toData<std::vector<byte> >(const std::vector<byte> *object) {
                    if (NULL == object) {
                        return Data();
                    }

                    DataOutput output;

                    // write partition hash
                    writeHash(output);

                    // write type
                    output.writeInt(SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY);

                    output.writeByteArray(object);

                    Data data(output.toByteArray());
                    return data;
                }

                template<>
                Data SerializationService::toData<std::vector<int16_t> >(const std::vector<int16_t> *object) {
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
                Data SerializationService::toData<std::vector<int32_t> >(const std::vector<int32_t> *object) {
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
                Data SerializationService::toData<std::vector<int64_t> >(const std::vector<int64_t> *object) {
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
                std::auto_ptr<Data> SerializationService::toObject(const Data *data) {
                    return std::auto_ptr<Data>((Data *) data);
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        return type;
                    }

                    type.typeId = data->getType();

                    // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                    // let usage of static member.
                    DataInput dataInput(data->toByteArray(), 4);

                    ObjectDataInput objectDataInput(dataInput, getSerializerHolder());

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                            SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        int32_t objectTypeId = objectDataInput.readInt();
                        assert(type.typeId == objectTypeId);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = objectDataInput.readBoolean();
                            if (!identified) {
                                throw exception::HazelcastSerializationException("SerializationService::getObjectType",
                                                                                 " DataSerializable is not identified");
                            }
                        }

                        type.factoryId = objectDataInput.readInt();
                        type.classId = objectDataInput.readInt();
                    }

                    return type;
                }

                void SerializationService::registerConstantSerializers() {
                    registerSerializer(boost::shared_ptr<SerializerBase>(new NullSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new DataSerializer(serializationConfig)));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new PortableSerializer(portableContext)));
                    //primitives and String
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::ByteSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::BooleanSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::CharSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::ShortSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::IntegerSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::LongSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::FloatSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::DoubleSerializer));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(boost::shared_ptr<SerializerBase>(new TheByteArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new BooleanArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new CharArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new ShortArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new IntegerArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new LongArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new FloatArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new DoubleArraySerializer()));
                    registerSerializer(boost::shared_ptr<SerializerBase>(new StringArraySerializer()));
                }
            }
        }
    }
}
