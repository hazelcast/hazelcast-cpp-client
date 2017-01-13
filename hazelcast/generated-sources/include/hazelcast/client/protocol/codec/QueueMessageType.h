/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API QueueMessageType {

                    HZ_QUEUE_OFFER=0x0301,
                    HZ_QUEUE_PUT=0x0302,
                    HZ_QUEUE_SIZE=0x0303,
                    HZ_QUEUE_REMOVE=0x0304,
                    HZ_QUEUE_POLL=0x0305,
                    HZ_QUEUE_TAKE=0x0306,
                    HZ_QUEUE_PEEK=0x0307,
                    HZ_QUEUE_ITERATOR=0x0308,
                    HZ_QUEUE_DRAINTO=0x0309,
                    HZ_QUEUE_DRAINTOMAXSIZE=0x030a,
                    HZ_QUEUE_CONTAINS=0x030b,
                    HZ_QUEUE_CONTAINSALL=0x030c,
                    HZ_QUEUE_COMPAREANDREMOVEALL=0x030d,
                    HZ_QUEUE_COMPAREANDRETAINALL=0x030e,
                    HZ_QUEUE_CLEAR=0x030f,
                    HZ_QUEUE_ADDALL=0x0310,
                    HZ_QUEUE_ADDLISTENER=0x0311,
                    HZ_QUEUE_REMOVELISTENER=0x0312,
                    HZ_QUEUE_REMAININGCAPACITY=0x0313,
                    HZ_QUEUE_ISEMPTY=0x0314
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE_H_




