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
// Created by sancar koyunlu on 9/4/13.



#include "hazelcast/client/queue/AddListenerRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            AddListenerRequest::AddListenerRequest(const std::string& name, bool includeValue)
            :name(name)
            , includeValue(includeValue) {
            }

            int AddListenerRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int AddListenerRequest::getClassId() const {
                return queue::QueuePortableHook::ADD_LISTENER;
            }

            void AddListenerRequest::write(serialization::PortableWriter& writer) const {
                writer.writeBoolean("l", false);
                writer.writeUTF("n", &name);
                writer.writeBoolean("i", includeValue);
            }
        }
    }
}

