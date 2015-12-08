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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_CONDITIONMESSAGETYPE_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_CONDITIONMESSAGETYPE_H_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum HAZELCAST_API ConditionMessageType {

                    HZ_CONDITION_AWAIT=0x0801,
                    HZ_CONDITION_BEFOREAWAIT=0x0802,
                    HZ_CONDITION_SIGNAL=0x0803,
                    HZ_CONDITION_SIGNALALL=0x0804
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_CONDITIONMESSAGETYPE_H_




