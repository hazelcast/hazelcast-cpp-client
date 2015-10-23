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



#include "hazelcast/client/lock/GetLockCountRequest.h"
#include "hazelcast/client/lock/LockPortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            GetLockCountRequest::GetLockCountRequest(serialization::pimpl::Data& key)
            :key(key) {
            }

            int GetLockCountRequest::getClassId() const {
                return LockPortableHook::GET_LOCK_COUNT;
            }

            int GetLockCountRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            }


            bool GetLockCountRequest::isRetryable() const {
                return true;
            }

            void GetLockCountRequest::write(serialization::PortableWriter& writer) const {
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&key);
            }
        }
    }
}

