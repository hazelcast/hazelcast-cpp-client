/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 05/04/15.
//


#pragma once
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "TestSerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace test {
            template <typename T>
            class ObjectCarryingPortable : public serialization::Portable {
            public:
                ObjectCarryingPortable(){

                }

                ObjectCarryingPortable(T* object):object(object){

                }

                int getFactoryId() const{
                    return TestSerializationConstants::TEST_DATA_FACTORY;
                }

                int getClassId() const{
                    return TestSerializationConstants::OBJECT_CARRYING_PORTABLE;
                }

                void writePortable(serialization::PortableWriter& writer) const{
                    serialization::ObjectDataOutput& output = writer.getRawDataOutput();
                    output.writeObject<T>(object.get());
                };

                void readPortable(serialization::PortableReader& reader){
                    serialization::ObjectDataInput& input = reader.getRawDataInput();
                    object = input.readObject<T>();
                };

                virtual bool operator ==(const ObjectCarryingPortable& m) const{
                    if (this == &m)
                        return true;
                    if ((*object) != *(m.object))
                        return false;
                    return true;
                }

                virtual bool operator !=(const ObjectCarryingPortable& m) const{
                    return !(*this == m);
                }

                std::shared_ptr<T> object;
            };
        }
    }
}

