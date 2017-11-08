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

                    std::vector<boost::shared_ptr<StreamSerializer> > const& serializers = serializationConfig.getSerializers();
                    std::vector<boost::shared_ptr<StreamSerializer> >::const_iterator it;
                    SerializerHolder& serializerHolder = getSerializerHolder();
                    for (it = serializers.begin(); it < serializers.end(); ++it) {
                        serializerHolder.registerSerializer(*it);
                    }
                }

                SerializerHolder& SerializationService::getSerializerHolder() {
                    return portableContext.getSerializerHolder();
                }

                bool SerializationService::registerSerializer(boost::shared_ptr<StreamSerializer> serializer) {
                    return getSerializerHolder().registerSerializer(serializer);
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 && data.getType() == SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void SerializationService::writeHash(DataOutput &out) {
                    out.writeInt(0);
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
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new NullSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new DataSerializer(serializationConfig)));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new PortableSerializer(portableContext)));
                    //primitives and String
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::ByteSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::BooleanSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::CharSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::ShortSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::IntegerSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::LongSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::FloatSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::DoubleSerializer));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new pimpl::StringSerializer));
                    //Arrays of primitives and String
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new TheByteArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new BooleanArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new CharArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new ShortArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new IntegerArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new LongArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new FloatArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new DoubleArraySerializer()));
                    registerSerializer(boost::shared_ptr<StreamSerializer>(new StringArraySerializer()));
                }
            }
        }
    }
}
