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
// Created by sancar koyunlu on 5/15/13.


#ifndef HAZELCAST_DATA_SERIALIZER
#define HAZELCAST_DATA_SERIALIZER

#include <memory>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/DataSerializableFactory.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        class SerializationConfig;

        namespace serialization {

            class ObjectDataOutput;

            class ObjectDataInput;

            namespace pimpl {
                class HAZELCAST_API DataSerializer : public StreamSerializer {
                public:
                    DataSerializer(const SerializationConfig &serializationConfig);

                    ~DataSerializer();

                    virtual int32_t getHazelcastTypeId() const;

                    template <typename T>
                    std::auto_ptr<T> readObject(ObjectDataInput &in) {
                        checkIfIdentifiedDataSerializable(in);
                        int32_t factoryId = readInt(in);
                        int32_t classId = readInt(in);

                        const std::map<int32_t, boost::shared_ptr<DataSerializableFactory> > &dataSerializableFactories =
                                serializationConfig.getDataSerializableFactories();
                        std::map<int, boost::shared_ptr<hazelcast::client::serialization::DataSerializableFactory> >::const_iterator dsfIterator =
                                dataSerializableFactories.find(factoryId);
                        std::auto_ptr<IdentifiedDataSerializable> object;

                        #ifdef __clang__
                        #pragma clang diagnostic push
                        #pragma clang diagnostic ignored "-Wreinterpret-base-class"
                        #endif
                        if (dsfIterator != dataSerializableFactories.end()) {
                            object = dsfIterator->second->create(classId);
                        } else {
                            object.reset(reinterpret_cast<IdentifiedDataSerializable *>(new T));
                        }

                        object->readData(in);

                        return std::auto_ptr<T>(reinterpret_cast<T *>(object.release()));
                        #ifdef __clang__
                        #pragma clang diagnostic pop
                        #endif
                    }

                    virtual void write(ObjectDataOutput &out, const IdentifiedDataSerializable *object);

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);

                private:
                    void checkIfIdentifiedDataSerializable(ObjectDataInput &in) const;

                    const SerializationConfig &serializationConfig;

                    int32_t readInt(ObjectDataInput &in) const;
                };
            }
        }
    }
}

#endif //HAZELCAST_DATA_SERIALIZER

