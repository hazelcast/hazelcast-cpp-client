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
// Created by sancar koyunlu on 9/5/13.



#include "hazelcast/client/topic/PortableMessage.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace topic {

            const serialization::pimpl::Data& PortableMessage::getMessage() const {
                return message;
            }

            const std::string & PortableMessage::getUuid() const {
                return *uuid;
            }

            long PortableMessage::getPublishTime() const {
                return publishTime;
            }

            int PortableMessage::getFactoryId() const {
                return TopicPortableHook::F_ID;
            }

            int PortableMessage::getClassId() const {
                return TopicPortableHook::PORTABLE_MESSAGE;
            }

            void PortableMessage::readPortable(serialization::PortableReader& reader) {
                publishTime = reader.readLong("pt");
                uuid = reader.readUTF("u");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                message = in.readData();
            }
        }
    }
}


