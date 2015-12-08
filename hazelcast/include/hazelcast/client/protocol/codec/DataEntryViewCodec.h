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
//
// Created by ihsan demir on 5/11/15.
//
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_ENTRYVIEWCODEC_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_ENTRYVIEWCODEC_

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class DataEntryView;
        }

        namespace protocol {
            class ClientMessage;

            namespace codec {
                class HAZELCAST_API DataEntryViewCodec {
                public:
                    static map::DataEntryView decode(ClientMessage &clientMessage);

                    static void encode(const map::DataEntryView &view, ClientMessage &clientMessage);

                    static int calculateDataSize(const map::DataEntryView &view);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_PROTOCOL_CODEC_ENTRYVIEWCODEC_

