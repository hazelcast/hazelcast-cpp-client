/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CLIENTPROTOCOLERRORCODES_H_
#define HAZELCAST_CLIENT_PROTOCOL_CLIENTPROTOCOLERRORCODES_H_

#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <map>

namespace hazelcast {
    namespace client {
        namespace protocol {
            enum ClientProtocolErrorCodes {
                UNDEFINED = 0,
                ARRAY_INDEX_OUT_OF_BOUNDS = 1,
                ARRAY_STORE = 2,
                AUTHENTICATIONERROR = 3,
                CACHE = 4,
                CACHE_LOADER = 5,
                CACHE_NOT_EXISTS = 6,
                CACHE_WRITER = 7,
                CALLER_NOT_MEMBER = 8,
                CANCELLATION = 9,
                CLASS_CAST = 10,
                CLASS_NOT_FOUND = 11,
                CONCURRENT_MODIFICATION = 12,
                CONFIG_MISMATCH = 13,
                CONFIGURATION = 14,
                DISTRIBUTED_OBJECT_DESTROYED = 15,
                DUPLICATE_INSTANCE_NAME = 16,
                ENDOFFILE = 17,
                ENTRY_PROCESSOR = 18,
                EXECUTION = 19,
                HAZELCAST = 20,
                HAZELCAST_INSTANCE_NOT_ACTIVE = 21,
                HAZELCAST_OVERLOAD = 22,
                HAZELCAST_SERIALIZATION = 23,
                IO = 24,
                ILLEGAL_ARGUMENT = 25,
                ILLEGAL_ACCESS_EXCEPTION = 26,
                ILLEGAL_ACCESS_ERROR = 27,
                ILLEGAL_MONITOR_STATE = 28,
                ILLEGAL_STATE = 29,
                ILLEGAL_THREAD_STATE = 30,
                INDEX_OUT_OF_BOUNDS = 31,
                INTERRUPTED = 32,
                INVALID_ADDRESS = 33,
                INVALID_CONFIGURATION = 34,
                MEMBER_LEFT = 35,
                NEGATIVE_ARRAY_SIZE = 36,
                NO_SUCH_ELEMENT = 37,
                NOT_SERIALIZABLE = 38,
                NULL_POINTER = 39,
                OPERATION_TIMEOUT = 40,
                PARTITION_MIGRATING = 41,
                QUERY = 42,
                QUERY_RESULT_SIZE_EXCEEDED = 43,
                QUORUM = 44,
                REACHED_MAX_SIZE = 45,
                REJECTED_EXECUTION = 46,
                REMOTE_MAP_REDUCE = 47,
                RESPONSE_ALREADY_SENT = 48,
                RETRYABLE_HAZELCAST = 49,
                RETRYABLE_IO = 50,
                RUNTIME = 51,
                SECURITY = 52,
                SOCKET = 53,
                STALE_SEQUENCE = 54,
                TARGET_DISCONNECTED = 55,
                TARGET_NOT_MEMBER = 56,
                TIMEOUT = 57,
                TOPIC_OVERLOAD = 58,
                TOPOLOGY_CHANGED = 59,
                TRANSACTION = 60,
                TRANSACTION_NOT_ACTIVE = 61,
                TRANSACTION_TIMED_OUT = 62,
                URI_SYNTAX = 63,
                UTF_DATA_FORMAT = 64,
                UNSUPPORTED_OPERATION = 65,
                WRONG_TARGET = 66,
                XA = 67,
                ACCESS_CONTROL = 68,
                LOGIN = 69,
                UNSUPPORTED_CALLBACK = 70,
                NO_DATA_MEMBER = 71,
                REPLICATED_MAP_CANT_BE_CREATED = 72,
                MAX_MESSAGE_SIZE_EXCEEDED = 73,
                WAN_REPLICATION_QUEUE_FULL = 74,
                ASSERTION_ERROR = 75,
                OUT_OF_MEMORY_ERROR = 76,
                STACK_OVERFLOW_ERROR = 77,
                NATIVE_OUT_OF_MEMORY_ERROR = 78,
                SERVICE_NOT_FOUND = 79
            };
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CLIENTPROTOCOLERRORCODES_H_
