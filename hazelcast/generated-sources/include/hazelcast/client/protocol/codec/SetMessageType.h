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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API SetMessageType {

                    HZ_SET_SIZE = 0x0601,
                    HZ_SET_CONTAINS = 0x0602,
                    HZ_SET_CONTAINSALL = 0x0603,
                    HZ_SET_ADD = 0x0604,
                    HZ_SET_REMOVE = 0x0605,
                    HZ_SET_ADDALL = 0x0606,
                    HZ_SET_COMPAREANDREMOVEALL = 0x0607,
                    HZ_SET_COMPAREANDRETAINALL = 0x0608,
                    HZ_SET_CLEAR = 0x0609,
                    HZ_SET_GETALL = 0x060a,
                    HZ_SET_ADDLISTENER = 0x060b,
                    HZ_SET_REMOVELISTENER = 0x060c,
                    HZ_SET_ISEMPTY = 0x060d
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE_H_




