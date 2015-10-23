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
// Created by sancar koyunlu on 5/15/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DataSerializer::DataSerializer() {
                }

                DataSerializer::~DataSerializer() {
                }

                void DataSerializer::write(ObjectDataOutput &out, const IdentifiedDataSerializable &object) const {
                    out.writeBoolean(true);
                    out.writeInt(object.getFactoryId());
                    out.writeInt(object.getClassId());
                    object.writeData(out);
                }

                void DataSerializer::read(ObjectDataInput &in, IdentifiedDataSerializable &object) const {
                    bool identified = in.readBoolean();
                    if (!identified) {
                        throw exception::HazelcastSerializationException("void DataSerializer::read", " DataSerializable is not identified");
                    }
                    in.readInt(); //factoryId
                    in.readInt(); //classId
                    object.readData(in);
                }

            }
        }
    }
}


