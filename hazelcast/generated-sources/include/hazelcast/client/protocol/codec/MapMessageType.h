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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API MapMessageType {

                    HZ_MAP_PUT=0x0101,
                    HZ_MAP_GET=0x0102,
                    HZ_MAP_REMOVE=0x0103,
                    HZ_MAP_REPLACE=0x0104,
                    HZ_MAP_REPLACEIFSAME=0x0105,
                    HZ_MAP_CONTAINSKEY=0x0109,
                    HZ_MAP_CONTAINSVALUE=0x010a,
                    HZ_MAP_REMOVEIFSAME=0x010b,
                    HZ_MAP_DELETE=0x010c,
                    HZ_MAP_FLUSH=0x010d,
                    HZ_MAP_TRYREMOVE=0x010e,
                    HZ_MAP_TRYPUT=0x010f,
                    HZ_MAP_PUTTRANSIENT=0x0110,
                    HZ_MAP_PUTIFABSENT=0x0111,
                    HZ_MAP_SET=0x0112,
                    HZ_MAP_LOCK=0x0113,
                    HZ_MAP_TRYLOCK=0x0114,
                    HZ_MAP_ISLOCKED=0x0115,
                    HZ_MAP_UNLOCK=0x0116,
                    HZ_MAP_ADDINTERCEPTOR=0x0117,
                    HZ_MAP_REMOVEINTERCEPTOR=0x0118,
                    HZ_MAP_ADDENTRYLISTENERTOKEYWITHPREDICATE=0x0119,
                    HZ_MAP_ADDENTRYLISTENERWITHPREDICATE=0x011a,
                    HZ_MAP_ADDENTRYLISTENERTOKEY=0x011b,
                    HZ_MAP_ADDENTRYLISTENER=0x011c,
                    HZ_MAP_ADDNEARCACHEENTRYLISTENER=0x011d,
                    HZ_MAP_REMOVEENTRYLISTENER=0x011e,
                    HZ_MAP_ADDPARTITIONLOSTLISTENER=0x011f,
                    HZ_MAP_REMOVEPARTITIONLOSTLISTENER=0x0120,
                    HZ_MAP_GETENTRYVIEW=0x0121,
                    HZ_MAP_EVICT=0x0122,
                    HZ_MAP_EVICTALL=0x0123,
                    HZ_MAP_LOADALL=0x0124,
                    HZ_MAP_LOADGIVENKEYS=0x0125,
                    HZ_MAP_KEYSET=0x0126,
                    HZ_MAP_GETALL=0x0127,
                    HZ_MAP_VALUES=0x0128,
                    HZ_MAP_ENTRYSET=0x0129,
                    HZ_MAP_KEYSETWITHPREDICATE=0x012a,
                    HZ_MAP_VALUESWITHPREDICATE=0x012b,
                    HZ_MAP_ENTRIESWITHPREDICATE=0x012c,
                    HZ_MAP_ADDINDEX=0x012d,
                    HZ_MAP_SIZE=0x012e,
                    HZ_MAP_ISEMPTY=0x012f,
                    HZ_MAP_PUTALL=0x0130,
                    HZ_MAP_CLEAR=0x0131,
                    HZ_MAP_EXECUTEONKEY=0x0132,
                    HZ_MAP_SUBMITTOKEY=0x0133,
                    HZ_MAP_EXECUTEONALLKEYS=0x0134,
                    HZ_MAP_EXECUTEWITHPREDICATE=0x0135,
                    HZ_MAP_EXECUTEONKEYS=0x0136,
                    HZ_MAP_FORCEUNLOCK=0x0137,
                    HZ_MAP_KEYSETWITHPAGINGPREDICATE=0x0138,
                    HZ_MAP_VALUESWITHPAGINGPREDICATE=0x0139,
                    HZ_MAP_ENTRIESWITHPAGINGPREDICATE=0x013a,
                    HZ_MAP_CLEARNEARCACHE=0x013b
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPMESSAGETYPE_H_




