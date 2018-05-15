/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/foreach.hpp>
#include <sstream>

#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializerHolder::SerializerHolder(const boost::shared_ptr<StreamSerializer> &globalSerializer)
                        : active(true), globalSerializer(globalSerializer) {
                }

                bool SerializerHolder::registerSerializer(const boost::shared_ptr<StreamSerializer> &serializer) {
                    boost::shared_ptr<SerializerBase> available = serializers.putIfAbsent(
                            serializer->getHazelcastTypeId(), serializer);
                    return available.get() == NULL;
                }

                boost::shared_ptr<StreamSerializer> SerializerHolder::serializerFor(int typeId) {
                    boost::shared_ptr<StreamSerializer> serializer = serializers.get(typeId);

                    if (serializer.get()) {
                        return serializer;
                    }

                    serializer = lookupGlobalSerializer(typeId);

                    if (!serializer.get()) {
                        if (active) {
                            std::ostringstream out;
                            out << "There is no suitable serializer for " << typeId;
                            throw exception::HazelcastSerializationException("SerializerHolder::registerSerializer",
                                                                             out.str());
                        }
                        throw exception::HazelcastClientNotActiveException("SerializerHolder::registerSerializer");
                    }
                    return serializer;
                }

                void SerializerHolder::dispose() {
                    active = false;

                    BOOST_FOREACH(boost::shared_ptr<StreamSerializer> serializer, serializers.values()) {
                                    serializer->destroy();
                                }

                    serializers.clear();
                }

                boost::shared_ptr<StreamSerializer> SerializerHolder::lookupGlobalSerializer(int typeId) {
                    if (!globalSerializer.get()) {
                        return boost::shared_ptr<StreamSerializer>();
                    }

                    std::ostringstream out;
                    out << "Registering global serializer for: " << typeId;
                    util::ILogger::getLogger().finest(out.str());
                    serializers.putIfAbsent(typeId, globalSerializer);
                    return globalSerializer;
                }

            }
        }
    }
}
