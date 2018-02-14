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

#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                map::DataEntryView DataEntryViewCodec::decode(ClientMessage &clientMessage) {
                    serialization::pimpl::Data key = clientMessage.get<serialization::pimpl::Data>(); // key
                    serialization::pimpl::Data value = clientMessage.get<serialization::pimpl::Data>(); // value
                    int64_t cost = clientMessage.get<int64_t>(); // cost
                    int64_t creationTime = clientMessage.get<int64_t>(); // creationTime
                    int64_t expirationTime = clientMessage.get<int64_t>(); // expirationTime
                    int64_t hits = clientMessage.get<int64_t>(); // hits
                    int64_t lastAccessTime = clientMessage.get<int64_t>(); // lastAccessTime
                    int64_t lastStoredTime = clientMessage.get<int64_t>(); // lastStoredTime
                    int64_t lastUpdateTime = clientMessage.get<int64_t>(); // lastUpdateTime
                    int64_t version = clientMessage.get<int64_t>(); // version
                    int64_t evictionCriteria = clientMessage.get<int64_t>(); // evictionCriteriaNumber
                    int64_t ttl = clientMessage.get<int64_t>();  // ttl
					return map::DataEntryView(key, value, cost, creationTime, expirationTime, hits, lastAccessTime,
						lastStoredTime, lastUpdateTime, version, evictionCriteria, ttl);
                }

                void DataEntryViewCodec::encode(const map::DataEntryView &view, ClientMessage &clientMessage) {
                    clientMessage.set(view.getKey());
                    clientMessage.set(view.getValue());
                    clientMessage.set((int64_t)view.getCost());
                    clientMessage.set((int64_t)view.getCreationTime());
                    clientMessage.set((int64_t)view.getExpirationTime());
                    clientMessage.set((int64_t)view.getHits());
                    clientMessage.set((int64_t)view.getLastAccessTime());
                    clientMessage.set((int64_t)view.getLastStoredTime());
                    clientMessage.set((int64_t)view.getLastUpdateTime());
                    clientMessage.set((int64_t)view.getVersion());
                    clientMessage.set((int64_t)view.getEvictionCriteriaNumber());
                    clientMessage.set((int64_t)view.getTtl());
                }

                int DataEntryViewCodec::calculateDataSize(const map::DataEntryView &view) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;;
                    return dataSize
                           + ClientMessage::calculateDataSize(view.getKey())
                           + ClientMessage::calculateDataSize(view.getValue())
                           + ClientMessage::INT64_SIZE * 10;
                }
            }
        }
    }
}
