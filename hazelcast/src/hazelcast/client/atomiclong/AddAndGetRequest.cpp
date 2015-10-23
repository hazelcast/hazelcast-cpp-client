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
// Created by sancar koyunlu on 19/11/13.



#include "hazelcast/client/atomiclong/AddAndGetRequest.h"
#include "hazelcast/client/atomiclong/AtomicLongPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            AddAndGetRequest::AddAndGetRequest(const std::string &instanceName, long delta)
            : AtomicLongRequest(instanceName, delta) {

            }

            int AddAndGetRequest::getClassId() const {
                return AtomicLongPortableHook::ADD_AND_GET;
            }

        }
    }
}



