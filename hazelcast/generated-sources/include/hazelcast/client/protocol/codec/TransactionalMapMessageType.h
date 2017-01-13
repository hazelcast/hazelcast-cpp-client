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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API TransactionalMapMessageType {

                    HZ_TRANSACTIONALMAP_CONTAINSKEY=0x1001,
                    HZ_TRANSACTIONALMAP_GET=0x1002,
                    HZ_TRANSACTIONALMAP_GETFORUPDATE=0x1003,
                    HZ_TRANSACTIONALMAP_SIZE=0x1004,
                    HZ_TRANSACTIONALMAP_ISEMPTY=0x1005,
                    HZ_TRANSACTIONALMAP_PUT=0x1006,
                    HZ_TRANSACTIONALMAP_SET=0x1007,
                    HZ_TRANSACTIONALMAP_PUTIFABSENT=0x1008,
                    HZ_TRANSACTIONALMAP_REPLACE=0x1009,
                    HZ_TRANSACTIONALMAP_REPLACEIFSAME=0x100a,
                    HZ_TRANSACTIONALMAP_REMOVE=0x100b,
                    HZ_TRANSACTIONALMAP_DELETE=0x100c,
                    HZ_TRANSACTIONALMAP_REMOVEIFSAME=0x100d,
                    HZ_TRANSACTIONALMAP_KEYSET=0x100e,
                    HZ_TRANSACTIONALMAP_KEYSETWITHPREDICATE=0x100f,
                    HZ_TRANSACTIONALMAP_VALUES=0x1010,
                    HZ_TRANSACTIONALMAP_VALUESWITHPREDICATE=0x1011
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE_H_




