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



#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/countdownlatch/CountDownLatchPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace countdownlatch {

            CountDownRequest::CountDownRequest(const std::string& instanceName)
            : instanceName(instanceName) {

            }

            int CountDownRequest::getFactoryId() const {
                return CountDownLatchPortableHook::F_ID;
            }

            int CountDownRequest::getClassId() const {
                return CountDownLatchPortableHook::COUNT_DOWN;
            }

            void CountDownRequest::write(serialization::PortableWriter & writer) const {
                writer.writeUTF("name", &instanceName);
            }

        }
    }
}
