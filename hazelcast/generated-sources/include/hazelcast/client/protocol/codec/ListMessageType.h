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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ListMessageType {

                    HZ_LIST_SIZE = 0x0501,
                    HZ_LIST_CONTAINS = 0x0502,
                    HZ_LIST_CONTAINSALL = 0x0503,
                    HZ_LIST_ADD = 0x0504,
                    HZ_LIST_REMOVE = 0x0505,
                    HZ_LIST_ADDALL = 0x0506,
                    HZ_LIST_COMPAREANDREMOVEALL = 0x0507,
                    HZ_LIST_COMPAREANDRETAINALL = 0x0508,
                    HZ_LIST_CLEAR = 0x0509,
                    HZ_LIST_GETALL = 0x050a,
                    HZ_LIST_ADDLISTENER = 0x050b,
                    HZ_LIST_REMOVELISTENER = 0x050c,
                    HZ_LIST_ISEMPTY = 0x050d,
                    HZ_LIST_ADDALLWITHINDEX = 0x050e,
                    HZ_LIST_GET = 0x050f,
                    HZ_LIST_SET = 0x0510,
                    HZ_LIST_ADDWITHINDEX = 0x0511,
                    HZ_LIST_REMOVEWITHINDEX = 0x0512,
                    HZ_LIST_LASTINDEXOF = 0x0513,
                    HZ_LIST_INDEXOF = 0x0514,
                    HZ_LIST_SUB = 0x0515,
                    HZ_LIST_ITERATOR = 0x0516,
                    HZ_LIST_LISTITERATOR = 0x0517
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTMESSAGETYPE_H_




