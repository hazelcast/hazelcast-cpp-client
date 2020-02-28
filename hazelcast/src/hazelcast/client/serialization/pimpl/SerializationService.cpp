/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include <ostream>
#include <cassert>

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ObjectType::ObjectType() : typeId(0), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << type.typeId << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig),
                          serializationConfig(serializationConfig) {
                    registerConstantSerializers();

                    std::vector<std::shared_ptr<SerializerBase> > const& serializers = serializationConfig.getSerializers();
                    for (std::vector<std::shared_ptr<SerializerBase> >::const_iterator it = serializers.begin();
                         it < serializers.end(); ++it) {
                        registerSerializer(std::static_pointer_cast<StreamSerializer>(*it));
                    }
                }

                SerializerHolder& SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }

                bool SerializationService::registerSerializer(std::shared_ptr<StreamSerializer> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
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
                    registerSerializer(std::shared_ptr<StreamSerializer>(new NullSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DataSerializer(serializationConfig)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new PortableSerializer(portableContext)));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new HazelcastJsonValueSerializer()));
                    //primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ByteSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::BooleanSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::CharSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::ShortSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::IntegerSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::LongSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::FloatSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::DoubleSerializer));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(std::shared_ptr<StreamSerializer>(new TheByteArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new BooleanArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new CharArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new ShortArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new IntegerArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new LongArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new FloatArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new DoubleArraySerializer()));
                    registerSerializer(std::shared_ptr<StreamSerializer>(new StringArraySerializer()));
                }

                void SerializationService::dispose() {
                    getSerializerHolder().dispose();
                }

                template <>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    const std::shared_ptr<Data> data = object->getData();
                    if ((Data *)NULL == data.get()) {
                        return Data();
                    }

                    return Data(*data);
                }

            }
        }
    }
}
