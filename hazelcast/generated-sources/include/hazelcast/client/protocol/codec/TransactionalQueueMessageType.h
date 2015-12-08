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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALQUEUEMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALQUEUEMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalQueueMessageType {

                    HZ_TRANSACTIONALQUEUE_OFFER=0x1401,
                    HZ_TRANSACTIONALQUEUE_TAKE=0x1402,
                    HZ_TRANSACTIONALQUEUE_POLL=0x1403,
                    HZ_TRANSACTIONALQUEUE_PEEK=0x1404,
                    HZ_TRANSACTIONALQUEUE_SIZE=0x1405
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALQUEUEMESSAGETYPE_H_




