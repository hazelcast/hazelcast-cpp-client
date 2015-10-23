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
// Created by sancar koyunlu on 9/10/13.



#include "hazelcast/client/multimap/AddEntryListenerRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, const serialization::pimpl::Data &key, bool includeValue)
            :name(name)
            , key(key)
            , hasKey(true)
            , includeValue(includeValue) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue)
            :name(name)
            , hasKey(false)
            , includeValue(includeValue) {

            }

            int AddEntryListenerRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int AddEntryListenerRequest::getClassId() const {
                return MultiMapPortableHook::ADD_ENTRY_LISTENER;
            }


            void AddEntryListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeBoolean("l", false);
                writer.writeBoolean("i", includeValue);
                writer.writeUTF("n", &name);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                
                if (hasKey) {
                    out.writeData(&key);
                } else {
                    out.writeData(NULL);
                }
            }

            bool AddEntryListenerRequest::isRetryable() const {
                return true;
            }

        }
    }
}


