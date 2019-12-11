/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_SEMAPHOREMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_SEMAPHOREMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API SemaphoreMessageType {

                    HZ_SEMAPHORE_INIT = 0x0d01,
                    HZ_SEMAPHORE_ACQUIRE = 0x0d02,
                    HZ_SEMAPHORE_AVAILABLEPERMITS = 0x0d03,
                    HZ_SEMAPHORE_DRAINPERMITS = 0x0d04,
                    HZ_SEMAPHORE_REDUCEPERMITS = 0x0d05,
                    HZ_SEMAPHORE_RELEASE = 0x0d06,
                    HZ_SEMAPHORE_TRYACQUIRE = 0x0d07,
                    HZ_SEMAPHORE_INCREASEPERMITS = 0x0d08
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_SEMAPHOREMESSAGETYPE_H_




