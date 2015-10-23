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



#include "hazelcast/client/queue/CompareAndRemoveRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace queue {
            CompareAndRemoveRequest::CompareAndRemoveRequest(const std::string& name, const std::vector<serialization::pimpl::Data>& dataList, bool retain)
            :name(name)
            , dataList(dataList)
            , retain(retain) {

            }

            int CompareAndRemoveRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int CompareAndRemoveRequest::getClassId() const {
                return queue::QueuePortableHook::COMPARE_AND_REMOVE;
            }

            void CompareAndRemoveRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", &name);
                writer.writeLong("t", 0);
                writer.writeBoolean("r", retain);
                int size = dataList.size();
                writer.writeInt("s", size);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                for (int i = 0; i < size; ++i) {
                    out.writeData(&(dataList[i]));
                }
            }
        }
    }
}

