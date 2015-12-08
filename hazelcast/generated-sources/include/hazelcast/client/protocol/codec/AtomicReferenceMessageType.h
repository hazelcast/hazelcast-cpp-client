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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICREFERENCEMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICREFERENCEMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API AtomicReferenceMessageType {

                    HZ_ATOMICREFERENCE_APPLY=0x0b01,
                    HZ_ATOMICREFERENCE_ALTER=0x0b02,
                    HZ_ATOMICREFERENCE_ALTERANDGET=0x0b03,
                    HZ_ATOMICREFERENCE_GETANDALTER=0x0b04,
                    HZ_ATOMICREFERENCE_CONTAINS=0x0b05,
                    HZ_ATOMICREFERENCE_COMPAREANDSET=0x0b06,
                    HZ_ATOMICREFERENCE_GET=0x0b08,
                    HZ_ATOMICREFERENCE_SET=0x0b09,
                    HZ_ATOMICREFERENCE_CLEAR=0x0b0a,
                    HZ_ATOMICREFERENCE_GETANDSET=0x0b0b,
                    HZ_ATOMICREFERENCE_SETANDGET=0x0b0c,
                    HZ_ATOMICREFERENCE_ISNULL=0x0b0d
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICREFERENCEMESSAGETYPE_H_




