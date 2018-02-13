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
//
// Created by ihsan demir on 5/11/15.
//

#include "hazelcast/client/protocol/codec/DistributedObjectInfoCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/impl/DistributedObjectInfo.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                impl::DistributedObjectInfo DistributedObjectInfoCodec::decode(ClientMessage &clientMessage) {
                    std::string serviceName = clientMessage.getStringUtf8();
                    std::string name = clientMessage.getStringUtf8();
                    return impl::DistributedObjectInfo(serviceName, name);
                }

                void DistributedObjectInfoCodec::encode(const impl::DistributedObjectInfo &info,
                                                        ClientMessage &clientMessage) {
                    clientMessage.set(info.getServiceName());
                    clientMessage.set(info.getName());
                }

                int DistributedObjectInfoCodec::calculateDataSize(const impl::DistributedObjectInfo &info) {
                    return ClientMessage::calculateDataSize(info.getServiceName()) +
                           ClientMessage::calculateDataSize(info.getName());
                }
            }
        }
    }
}
