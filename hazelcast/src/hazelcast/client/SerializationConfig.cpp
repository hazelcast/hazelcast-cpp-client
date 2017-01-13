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
// Created by sancar koyunlu on 25/06/14.
//

#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {

        SerializationConfig::SerializationConfig() : version(0) {

        }

        int SerializationConfig::getPortableVersion() const {
            return version;
        }

        SerializationConfig &SerializationConfig::setPortableVersion(int version) {
            this->version = version;
            return *this;
        }

        std::vector<boost::shared_ptr<serialization::SerializerBase> > const &SerializationConfig::getSerializers() const {
            return serializers;
        }

        SerializationConfig &SerializationConfig::registerSerializer(boost::shared_ptr<serialization::SerializerBase> serializer) {
            serializers.push_back(serializer);
            return *this;
        }
    }
}

