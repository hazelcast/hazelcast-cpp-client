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



#include "hazelcast/client/topic/PublishRequest.h"
#include "hazelcast/client/topic/TopicPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            PublishRequest::PublishRequest(const std::string& instanceName, const serialization::pimpl::Data& message)
            : instanceName(instanceName)
            , message(message) {

            }

            int PublishRequest::getFactoryId() const {
                return TopicPortableHook::F_ID;
            }

            int PublishRequest::getClassId() const {
                return TopicPortableHook::PUBLISH;
            }


            void PublishRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &instanceName);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&message);
            }
        }
    }
}


