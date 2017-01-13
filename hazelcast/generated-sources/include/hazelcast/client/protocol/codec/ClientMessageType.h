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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ClientMessageType {

                    HZ_CLIENT_AUTHENTICATION=0x0002,
                    HZ_CLIENT_AUTHENTICATIONCUSTOM=0x0003,
                    HZ_CLIENT_ADDMEMBERSHIPLISTENER=0x0004,
                    HZ_CLIENT_CREATEPROXY=0x0005,
                    HZ_CLIENT_DESTROYPROXY=0x0006,
                    HZ_CLIENT_GETPARTITIONS=0x0008,
                    HZ_CLIENT_REMOVEALLLISTENERS=0x0009,
                    HZ_CLIENT_ADDPARTITIONLOSTLISTENER=0x000a,
                    HZ_CLIENT_REMOVEPARTITIONLOSTLISTENER=0x000b,
                    HZ_CLIENT_GETDISTRIBUTEDOBJECTS=0x000c,
                    HZ_CLIENT_ADDDISTRIBUTEDOBJECTLISTENER=0x000d,
                    HZ_CLIENT_REMOVEDISTRIBUTEDOBJECTLISTENER=0x000e,
                    HZ_CLIENT_PING=0x000f
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE_H_




