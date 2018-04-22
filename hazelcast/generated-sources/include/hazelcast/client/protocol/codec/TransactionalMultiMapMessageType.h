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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMULTIMAPMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMULTIMAPMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalMultiMapMessageType {

                    HZ_TRANSACTIONALMULTIMAP_PUT = 0x1101,
                    HZ_TRANSACTIONALMULTIMAP_GET = 0x1102,
                    HZ_TRANSACTIONALMULTIMAP_REMOVE = 0x1103,
                    HZ_TRANSACTIONALMULTIMAP_REMOVEENTRY = 0x1104,
                    HZ_TRANSACTIONALMULTIMAP_VALUECOUNT = 0x1105,
                    HZ_TRANSACTIONALMULTIMAP_SIZE = 0x1106
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMULTIMAPMESSAGETYPE_H_




