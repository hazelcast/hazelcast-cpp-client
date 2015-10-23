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



#include "hazelcast/client/map/ReplaceRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            ReplaceRequest::ReplaceRequest(const std::string& name, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long threadId)
            : name(name)
            , key(key)
            , value(value)
            , threadId(threadId) {

            }

            int ReplaceRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ReplaceRequest::getClassId() const {
                return PortableHook::REPLACE;
            }

            void ReplaceRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
                writer.writeLong("t", threadId);
                writer.writeLong("ttl", -1);
                writer.writeBoolean("a", false);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&key);
                out.writeData(&value);
            }


        }
    }
}


