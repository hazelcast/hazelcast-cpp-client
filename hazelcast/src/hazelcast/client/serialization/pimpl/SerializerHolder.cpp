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
// Created by sancar koyunlu on 7/31/13.



#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/Serializer.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializerHolder::SerializerHolder(PortableContext&context)
                :portableSerializer(context) {

                }

                bool SerializerHolder::registerSerializer(boost::shared_ptr<SerializerBase> serializer) {
                    boost::shared_ptr<SerializerBase> available = serializers.putIfAbsent(serializer->getHazelcastTypeId(), serializer);
                    return available.get() == NULL;
                }

                boost::shared_ptr<SerializerBase> SerializerHolder::serializerFor(int typeId) {
                    return serializers.get(typeId);
                }

                PortableSerializer &SerializerHolder::getPortableSerializer() {
                    return portableSerializer;
                }

                DataSerializer &SerializerHolder::getDataSerializer() {
                    return dataSerializer;
                }
            }
        }
    }
}
