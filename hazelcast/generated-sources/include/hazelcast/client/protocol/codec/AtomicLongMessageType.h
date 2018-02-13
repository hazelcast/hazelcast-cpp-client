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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API AtomicLongMessageType {

                    HZ_ATOMICLONG_APPLY=0x0a01,
                    HZ_ATOMICLONG_ALTER=0x0a02,
                    HZ_ATOMICLONG_ALTERANDGET=0x0a03,
                    HZ_ATOMICLONG_GETANDALTER=0x0a04,
                    HZ_ATOMICLONG_ADDANDGET=0x0a05,
                    HZ_ATOMICLONG_COMPAREANDSET=0x0a06,
                    HZ_ATOMICLONG_DECREMENTANDGET=0x0a07,
                    HZ_ATOMICLONG_GET=0x0a08,
                    HZ_ATOMICLONG_GETANDADD=0x0a09,
                    HZ_ATOMICLONG_GETANDSET=0x0a0a,
                    HZ_ATOMICLONG_INCREMENTANDGET=0x0a0b,
                    HZ_ATOMICLONG_GETANDINCREMENT=0x0a0c,
                    HZ_ATOMICLONG_SET=0x0a0d
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE_H_




