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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_PNCOUNTERMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_PNCOUNTERMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API PNCounterMessageType {

                    HZ_PNCOUNTER_GET=0x2001,
                    HZ_PNCOUNTER_ADD=0x2002,
                    HZ_PNCOUNTER_GETCONFIGUREDREPLICACOUNT=0x2003
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_PNCOUNTERMESSAGETYPE_H_




