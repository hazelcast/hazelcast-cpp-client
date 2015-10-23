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
// Created by sancar koyunlu on 9/11/13.



#include "hazelcast/client/multimap/MultiMapLockRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapLockRequest::MultiMapLockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId)
            :KeyBasedRequest(name, key)
            , threadId(threadId)
            , ttl(-1)
            , timeout(-1) {

            }

            MultiMapLockRequest::MultiMapLockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, long ttl, long timeout)
            :KeyBasedRequest(name, key)
            , threadId(threadId)
            , ttl(ttl)
            , timeout(timeout) {

            }

            int MultiMapLockRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int MultiMapLockRequest::getClassId() const {
                return MultiMapPortableHook::LOCK;
            }

            void MultiMapLockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                writer.writeLong("ttl", ttl);
                writer.writeLong("timeout", timeout);
                KeyBasedRequest::write(writer);
            }

        }
    }
}
