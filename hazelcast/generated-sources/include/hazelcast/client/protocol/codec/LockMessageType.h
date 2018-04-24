/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_LOCKMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_LOCKMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API LockMessageType {

                    HZ_LOCK_ISLOCKED = 0x0701,
                    HZ_LOCK_ISLOCKEDBYCURRENTTHREAD = 0x0702,
                    HZ_LOCK_GETLOCKCOUNT = 0x0703,
                    HZ_LOCK_GETREMAININGLEASETIME = 0x0704,
                    HZ_LOCK_LOCK = 0x0705,
                    HZ_LOCK_UNLOCK = 0x0706,
                    HZ_LOCK_FORCEUNLOCK = 0x0707,
                    HZ_LOCK_TRYLOCK = 0x0708
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_LOCKMESSAGETYPE_H_




