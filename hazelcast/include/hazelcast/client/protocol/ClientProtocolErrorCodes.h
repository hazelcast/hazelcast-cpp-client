/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
            enum client_protocol_error_codes {
                UNDEFINED = 0,
                ARRAY_INDEX_OUT_OF_BOUNDS = 1,
                ARRAY_STORE = 2,
                AUTHENTICATION = 3,
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
                DISTRIBUTED_OBJECT_DESTROYED = 14,
                ENDOFFILE = 15,
                ENTRY_PROCESSOR = 16,
                EXECUTION = 17,
                HAZELCAST = 18,
                HAZELCAST_INSTANCE_NOT_ACTIVE = 19,
                HAZELCAST_OVERLOAD = 20,
                HAZELCAST_SERIALIZATION = 21,
                IO = 22,
                ILLEGAL_ARGUMENT = 23,
                ILLEGAL_ACCESS_EXCEPTION = 24,
                ILLEGAL_ACCESS_ERROR = 25,
                ILLEGAL_MONITOR_STATE = 26,
                ILLEGAL_STATE = 27,
                ILLEGAL_THREAD_STATE = 28,
                INDEX_OUT_OF_BOUNDS = 29,
                INTERRUPTED = 30,
                INVALID_ADDRESS = 31,
                INVALID_CONFIGURATION = 32,
                MEMBER_LEFT = 33,
                NEGATIVE_ARRAY_SIZE = 34,
                NO_SUCH_ELEMENT = 35,
                NOT_SERIALIZABLE = 36,
                NULL_POINTER = 37,
                OPERATION_TIMEOUT = 38,
                PARTITION_MIGRATING = 39,
                QUERY = 40,
                QUERY_RESULT_SIZE_EXCEEDED = 41,
                SPLIT_BRAIN_PROTECTION = 42,
                REACHED_MAX_SIZE = 43,
                REJECTED_EXECUTION = 44,
                RESPONSE_ALREADY_SENT = 45,
                RETRYABLE_HAZELCAST = 46,
                RETRYABLE_IO = 47,
                RUNTIME = 48,
                SECURITY = 49,
                SOCK_ERROR = 50,
                STALE_SEQUENCE = 51,
                TARGET_DISCONNECTED = 52,
                TARGET_NOT_MEMBER = 53,
                TIMEOUT = 54,
                TOPIC_OVERLOAD = 55,
                TRANSACTION = 56,
                TRANSACTION_NOT_ACTIVE = 57,
                TRANSACTION_TIMED_OUT = 58,
                URI_SYNTAX = 59,
                UTF_DATA_FORMAT = 60,
                UNSUPPORTED_OPERATION = 61,
                WRONG_TARGET = 62,
                XA = 63,
                ACCESS_CONTROL = 64,
                LOGIN = 65,
                UNSUPPORTED_CALLBACK = 66,
                NO_DATA_MEMBER = 67,
                REPLICATED_MAP_CANT_BE_CREATED = 68,
                MAX_MESSAGE_SIZE_EXCEEDED = 69,
                WAN_REPLICATION_QUEUE_FULL = 70,
                ASSERTION_ERROR = 71,
                OUT_OF_MEMORY_ERROR = 72,
                STACK_OVERFLOW_ERROR = 73,
                NATIVE_OUT_OF_MEMORY_ERROR = 74,
                SERVICE_NOT_FOUND = 75,
                STALE_TASK_ID = 76,
                DUPLICATE_TASK = 77,
                STALE_TASK = 78,
                LOCAL_MEMBER_RESET = 79,
                INDETERMINATE_OPERATION_STATE = 80,
                FLAKE_ID_NODE_ID_OUT_OF_RANGE_EXCEPTION = 81,
                TARGET_NOT_REPLICA_EXCEPTION = 82,
                MUTATION_DISALLOWED_EXCEPTION = 83,
                CONSISTENCY_LOST_EXCEPTION = 84,
                SESSION_EXPIRED_EXCEPTION = 85,
                WAIT_KEY_CANCELLED_EXCEPTION = 86,
                LOCK_ACQUIRE_LIMIT_REACHED_EXCEPTION = 87,
                LOCK_OWNERSHIP_LOST_EXCEPTION = 88,
                CP_GROUP_DESTROYED_EXCEPTION = 89,
                CANNOT_REPLICATE_EXCEPTION = 90,
                LEADER_DEMOTED_EXCEPTION = 91,
                STALE_APPEND_REQUEST_EXCEPTION = 92,
                NOT_LEADER_EXCEPTION = 93,
                VERSION_MISMATCH_EXCEPTION = 94,

                // Non protocol exceptions, thrown locally
                UNEXPECTED_MESSAGE_TYPE = -1,
                HAZELCAST_CLIENT_OFFLINE = -2,
                UNKNOWN_HOST = -3,
                CLIENT_NOT_ALLOWED_IN_CLUSTER = -4
            };
        }
    }
}


