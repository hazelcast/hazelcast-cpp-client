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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_RINGBUFFERMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_RINGBUFFERMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API RingbufferMessageType {

                    HZ_RINGBUFFER_SIZE = 0x1901,
                    HZ_RINGBUFFER_TAILSEQUENCE = 0x1902,
                    HZ_RINGBUFFER_HEADSEQUENCE = 0x1903,
                    HZ_RINGBUFFER_CAPACITY = 0x1904,
                    HZ_RINGBUFFER_REMAININGCAPACITY = 0x1905,
                    HZ_RINGBUFFER_ADD = 0x1906,
                    HZ_RINGBUFFER_READONE = 0x1908,
                    HZ_RINGBUFFER_ADDALL = 0x1909,
                    HZ_RINGBUFFER_READMANY = 0x190a
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_RINGBUFFERMESSAGETYPE_H_




