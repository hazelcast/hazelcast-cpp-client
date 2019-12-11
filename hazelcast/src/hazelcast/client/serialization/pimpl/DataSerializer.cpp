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

#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {
                }

                DataSerializer::~DataSerializer() {
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

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable *dataSerializable) {
                    out.writeBoolean(true);
                    out.writeInt(dataSerializable->getFactoryId());
                    out.writeInt(dataSerializable->getClassId());
                    dataSerializable->writeData(out);
                }

                void DataSerializer::write(ObjectDataOutput &out, const void *object) {
                    // should not be called
                    assert(0);
                }

                void *DataSerializer::read(ObjectDataInput &in) {
                    // should not be called
                    assert(0);
                    return NULL;
                }

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }
            }
        }
    }
}


