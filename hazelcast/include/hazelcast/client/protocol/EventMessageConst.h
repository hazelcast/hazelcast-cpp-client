/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
#pragma once

namespace hazelcast {
namespace client {
namespace protocol {
enum EventMessageConst
{
    EVENT_MEMBER = 200,
    EVENT_MEMBERLIST = 201,
    EVENT_MEMBERATTRIBUTECHANGE = 202,
    EVENT_ENTRY = 203,
    EVENT_ITEM = 204,
    EVENT_TOPIC = 205,
    EVENT_PARTITIONLOST = 206,
    EVENT_DISTRIBUTEDOBJECT = 207,
    EVENT_CACHEINVALIDATION = 208,
    EVENT_MAPPARTITIONLOST = 209,
    EVENT_CACHE = 210,
    EVENT_CACHEBATCHINVALIDATION = 211,

    // ENTERPRISE
    EVENT_QUERYCACHESINGLE = 212,
    EVENT_QUERYCACHEBATCH = 213,

    EVENT_CACHEPARTITIONLOST = 214,
    EVENT_IMAPINVALIDATION = 215,
    EVENT_IMAPBATCHINVALIDATION = 216,
    EVENT_PARTITIONS = 217
};
}
} // namespace client
} // namespace hazelcast
