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
// Created by sancar koyunlu on 7/31/13.


#ifndef HAZELCAST_SerializerHolder
#define HAZELCAST_SerializerHolder

#include "hazelcast/util/Disposable.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/SynchronizedMap.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class StreamSerializer;

            namespace pimpl {
                class HAZELCAST_API SerializerHolder : public util::Disposable {

                public:
                    SerializerHolder(const boost::shared_ptr<StreamSerializer> &globalSerializer);

                    bool registerSerializer(const boost::shared_ptr<StreamSerializer> &serializer);

                    boost::shared_ptr<StreamSerializer> serializerFor(int typeId);

                    virtual void dispose();

                private:
                    boost::shared_ptr<StreamSerializer> lookupGlobalSerializer(int typeId);

                    hazelcast::util::SynchronizedMap<int, StreamSerializer> serializers;
                    util::AtomicBoolean active;
                    const boost::shared_ptr<StreamSerializer> globalSerializer;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_SerializerHolder

