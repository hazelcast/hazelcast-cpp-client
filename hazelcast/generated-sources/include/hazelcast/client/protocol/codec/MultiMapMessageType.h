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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API MultiMapMessageType {

                    HZ_MULTIMAP_PUT = 0x0201,
                    HZ_MULTIMAP_GET = 0x0202,
                    HZ_MULTIMAP_REMOVE = 0x0203,
                    HZ_MULTIMAP_KEYSET = 0x0204,
                    HZ_MULTIMAP_VALUES = 0x0205,
                    HZ_MULTIMAP_ENTRYSET = 0x0206,
                    HZ_MULTIMAP_CONTAINSKEY = 0x0207,
                    HZ_MULTIMAP_CONTAINSVALUE = 0x0208,
                    HZ_MULTIMAP_CONTAINSENTRY = 0x0209,
                    HZ_MULTIMAP_SIZE = 0x020a,
                    HZ_MULTIMAP_CLEAR = 0x020b,
                    HZ_MULTIMAP_VALUECOUNT = 0x020c,
                    HZ_MULTIMAP_ADDENTRYLISTENERTOKEY = 0x020d,
                    HZ_MULTIMAP_ADDENTRYLISTENER = 0x020e,
                    HZ_MULTIMAP_REMOVEENTRYLISTENER = 0x020f,
                    HZ_MULTIMAP_LOCK = 0x0210,
                    HZ_MULTIMAP_TRYLOCK = 0x0211,
                    HZ_MULTIMAP_ISLOCKED = 0x0212,
                    HZ_MULTIMAP_UNLOCK = 0x0213,
                    HZ_MULTIMAP_FORCEUNLOCK = 0x0214,
                    HZ_MULTIMAP_REMOVEENTRY = 0x0215,
                    HZ_MULTIMAP_DELETE = 0x0216
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE_H_




