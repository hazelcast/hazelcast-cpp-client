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
//
// Created by ihsan demir on 5/11/15.
//

#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                Address AddressCodec::decode(ClientMessage &clientMessage) {
                    std::string host = clientMessage.getStringUtf8();
                    int32_t port = clientMessage.getInt32();
                    return Address(host, port);
                }

                void AddressCodec::encode(const Address &address, ClientMessage &clientMessage) {
                    clientMessage.set(address.getHost());
                    clientMessage.set((int32_t)address.getPort());
                }

                int AddressCodec::calculateDataSize(const Address &address) {
                    return ClientMessage::calculateDataSize(address.getHost()) + ClientMessage::INT32_SIZE;
                }
            }
        }
    }
}
