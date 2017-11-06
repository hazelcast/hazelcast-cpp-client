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

#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/DataSerializableFactory.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {
                }

                DataSerializer::~DataSerializer() {
                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable &object) {
                    out.writeBoolean(true);
                    out.writeInt(object.getFactoryId());
                    out.writeInt(object.getClassId());
                    object.writeData(out);
                }

                void DataSerializer::read(ObjectDataInput &in, IdentifiedDataSerializable &object) {
                    object.readData(in);
                }

                void DataSerializer::checkIfIdentifiedDataSerializable(ObjectDataInput &in) const {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read", " DataSerializable is not identified");
                    }
                }

                int32_t DataSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DATA;
                }

                void *DataSerializer::create(ObjectDataInput &in) {
                    // we read these three fields first so that if the other version of read method is called for
                    // backward compatibility, these fields will not be read again.
                    checkIfIdentifiedDataSerializable(in);
                    int32_t factoryId = in.readInt();
                    int32_t classId = in.readInt();

                    const std::map<int32_t, boost::shared_ptr<DataSerializableFactory> > &dataSerializableFactories =
                            serializationConfig.getDataSerializableFactories();
                    std::map<int, boost::shared_ptr<hazelcast::client::serialization::DataSerializableFactory> >::const_iterator dsfIterator =
                            dataSerializableFactories.find(factoryId);
                    if (dsfIterator == dataSerializableFactories.end()) {
                        // keep backward compatible, do not throw exception
                        return NULL;
                    }

                    return dsfIterator->second->create(classId).release();
                }
            }
        }
    }
}


