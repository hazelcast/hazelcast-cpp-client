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
// Created by sancar koyunlu on 6/25/13.


#include "hazelcast/client/lock/IsLockedRequest.h"
#include "hazelcast/client/lock/LockPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            IsLockedRequest::IsLockedRequest(serialization::pimpl::Data &key)
            : threadId(0)
            , key(key) {
            }

            IsLockedRequest::IsLockedRequest(serialization::pimpl::Data &key, long threadId)
            : threadId(threadId)
            , key(key) {
            }

            int IsLockedRequest::getClassId() const {
                return LockPortableHook::IS_LOCKED;
            }

            int IsLockedRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            }

            void IsLockedRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&key);
            }


            bool IsLockedRequest::isRetryable() const {
                return true;
            }

        }
    }
}


