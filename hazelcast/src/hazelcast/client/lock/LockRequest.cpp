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
// Created by sancar koyunlu on 8/12/13.



#include "hazelcast/client/lock/LockRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/lock/LockPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace lock {

            LockRequest::LockRequest(serialization::pimpl::Data &key, long threadId, long ttl, long timeout)
            :key(key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {
            }

            int LockRequest::getClassId() const {
                return LockPortableHook::LOCK;
            }

            int LockRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            }


            void LockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&key);
            }

        }
    }
}

