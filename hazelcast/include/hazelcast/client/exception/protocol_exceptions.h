/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include "hazelcast/client/exception/iexception.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
#define DEFINE_EXCEPTION_CLASS(ClassName, errorNo, isRuntime) \
            class HAZELCAST_API ClassName : public iexception {\
            public:\
                ClassName(const std::string& source, const std::string& message, \
                        const std::string& details = "", std::exception_ptr cause = nullptr, bool retryable = false) \
                    : ClassName(#ClassName, errorNo, source, message, details, cause, isRuntime, retryable) {}\
                explicit ClassName(const std::string& message) : ClassName("", message) {}\
                ClassName() : ClassName("", "") {}\
                ClassName(const std::string& errorName, int32_t errorCode, const std::string& source, const std::string& message, \
                        const std::string& details, std::exception_ptr cause, bool runtime, bool retryable) \
                    : iexception(errorName, source, message, details, errorCode, cause, runtime, retryable) {}\
            };\

            // ---------  Non-runtime starts here -------------------------------------------/
            DEFINE_EXCEPTION_CLASS(undefined_error_code , protocol::UNDEFINED, false);

            DEFINE_EXCEPTION_CLASS(execution , protocol::EXECUTION, false);

            DEFINE_EXCEPTION_CLASS(class_not_found , protocol::CLASS_NOT_FOUND, false);

            DEFINE_EXCEPTION_CLASS(eof , protocol::ENDOFFILE, false);

            DEFINE_EXCEPTION_CLASS(io , protocol::IO, false);

            DEFINE_EXCEPTION_CLASS(illegal_access , protocol::ILLEGAL_ACCESS_EXCEPTION, false);

            DEFINE_EXCEPTION_CLASS(illegal_access_error, protocol::ILLEGAL_ACCESS_ERROR, false);

            DEFINE_EXCEPTION_CLASS(interrupted , protocol::INTERRUPTED, false);

            DEFINE_EXCEPTION_CLASS(not_serializable , protocol::NOT_SERIALIZABLE, false);

            DEFINE_EXCEPTION_CLASS(socket , protocol::SOCK_ERROR, false);

            DEFINE_EXCEPTION_CLASS(timeout , protocol::TIMEOUT, false);
            DEFINE_EXCEPTION_CLASS(uri_syntax , protocol::URI_SYNTAX, false);
            DEFINE_EXCEPTION_CLASS(utf_data_format , protocol::UTF_DATA_FORMAT, false);
            DEFINE_EXCEPTION_CLASS(xa , protocol::XA, false);
            DEFINE_EXCEPTION_CLASS(login , protocol::LOGIN, false);
            DEFINE_EXCEPTION_CLASS(unsupported_callback , protocol::UNSUPPORTED_CALLBACK, false);
            DEFINE_EXCEPTION_CLASS(assertion_error, protocol::ASSERTION_ERROR, false);
            DEFINE_EXCEPTION_CLASS(out_of_memory_error, protocol::OUT_OF_MEMORY_ERROR, false);
            DEFINE_EXCEPTION_CLASS(stack_overflow_error, protocol::STACK_OVERFLOW_ERROR, false);
            DEFINE_EXCEPTION_CLASS(native_out_of_memory_error, protocol::NATIVE_OUT_OF_MEMORY_ERROR, false);
            // ---------  Non-runtime ends here -------------------------------------------/

            // ---------  runtime starts here -------------------------------------------/
            DEFINE_EXCEPTION_CLASS(array_index_out_of_bounds , protocol::INDEX_OUT_OF_BOUNDS, true);
            DEFINE_EXCEPTION_CLASS(array_store , protocol::ARRAY_STORE, true);
            DEFINE_EXCEPTION_CLASS(authentication , protocol::AUTHENTICATION, true);
            DEFINE_EXCEPTION_CLASS(cache_not_exists , protocol::CACHE_NOT_EXISTS, true);
            DEFINE_EXCEPTION_CLASS(cancellation , protocol::CANCELLATION, true);
            DEFINE_EXCEPTION_CLASS(class_cast , protocol::CLASS_CAST, true);
            DEFINE_EXCEPTION_CLASS(concurrent_modification , protocol::CONCURRENT_MODIFICATION, true);
            DEFINE_EXCEPTION_CLASS(config_mismatch , protocol::CONFIG_MISMATCH, true);
            DEFINE_EXCEPTION_CLASS(distributed_object_destroyed , protocol::DISTRIBUTED_OBJECT_DESTROYED, true);
            DEFINE_EXCEPTION_CLASS(entry_processor , protocol::ENTRY_PROCESSOR, true);
            DEFINE_EXCEPTION_CLASS(hazelcast_, protocol::HAZELCAST, true);
            DEFINE_EXCEPTION_CLASS(hazelcast_instance_not_active , protocol::HAZELCAST_INSTANCE_NOT_ACTIVE, true);
            DEFINE_EXCEPTION_CLASS(hazelcast_overload , protocol::HAZELCAST_OVERLOAD, true);
            DEFINE_EXCEPTION_CLASS(hazelcast_serialization , protocol::HAZELCAST_SERIALIZATION, true);
            DEFINE_EXCEPTION_CLASS(illegal_argument , protocol::ILLEGAL_ARGUMENT, true);
            DEFINE_EXCEPTION_CLASS(illegal_monitor_state , protocol::ILLEGAL_MONITOR_STATE, true);
            DEFINE_EXCEPTION_CLASS(illegal_state , protocol::ILLEGAL_STATE, true);
            DEFINE_EXCEPTION_CLASS(illegal_thread_state , protocol::ILLEGAL_THREAD_STATE, true);
            DEFINE_EXCEPTION_CLASS(index_out_of_bounds , protocol::INDEX_OUT_OF_BOUNDS, true);
            DEFINE_EXCEPTION_CLASS(invalid_address , protocol::INVALID_ADDRESS, true);
            DEFINE_EXCEPTION_CLASS(invalid_configuration , protocol::INVALID_CONFIGURATION, true);
            DEFINE_EXCEPTION_CLASS(negative_array_size , protocol::NEGATIVE_ARRAY_SIZE, true);
            DEFINE_EXCEPTION_CLASS(no_such_element , protocol::NO_SUCH_ELEMENT, true);
            DEFINE_EXCEPTION_CLASS(null_pointer , protocol::NULL_POINTER, true);
            DEFINE_EXCEPTION_CLASS(operation_timeout , protocol::OPERATION_TIMEOUT, true);
            DEFINE_EXCEPTION_CLASS(query , protocol::QUERY, true);
            DEFINE_EXCEPTION_CLASS(query_result_size_exceeded , protocol::QUERY_RESULT_SIZE_EXCEEDED, true);
            DEFINE_EXCEPTION_CLASS(split_brain_protection , protocol::SPLIT_BRAIN_PROTECTION, true);
            DEFINE_EXCEPTION_CLASS(reached_max_size , protocol::REACHED_MAX_SIZE, true);
            DEFINE_EXCEPTION_CLASS(rejected_execution , protocol::REJECTED_EXECUTION, true);
            DEFINE_EXCEPTION_CLASS(response_already_sent , protocol::RESPONSE_ALREADY_SENT, true);
            DEFINE_EXCEPTION_CLASS(runtime , protocol::RUNTIME, true);
            DEFINE_EXCEPTION_CLASS(SecurityException, protocol::SECURITY, true);
            /**
            * Raised at Ringbuffer::readOne if the provided sequence is smaller then Ringbuffer::headSequence() is passed.
            */
            DEFINE_EXCEPTION_CLASS(stale_sequence , protocol::STALE_SEQUENCE, true);
            DEFINE_EXCEPTION_CLASS(target_disconnected , protocol::TARGET_DISCONNECTED, true);
            DEFINE_EXCEPTION_CLASS(topic_overload , protocol::TOPIC_OVERLOAD, true);

            DEFINE_EXCEPTION_CLASS(transaction , protocol::TRANSACTION, true);
            DEFINE_EXCEPTION_CLASS(transaction_not_active , protocol::TRANSACTION_NOT_ACTIVE, true);
            DEFINE_EXCEPTION_CLASS(transaction_timed_out , protocol::TRANSACTION_TIMED_OUT, true);
            DEFINE_EXCEPTION_CLASS(unsupported_operation , protocol::UNSUPPORTED_OPERATION, true);
            DEFINE_EXCEPTION_CLASS(access_control , protocol::ACCESS_CONTROL, true);
            DEFINE_EXCEPTION_CLASS(no_data_member_in_cluster, protocol::NO_DATA_MEMBER, true);
            DEFINE_EXCEPTION_CLASS(replicated_map_cant_be_created_on_lite_member, protocol::REPLICATED_MAP_CANT_BE_CREATED, true);
            DEFINE_EXCEPTION_CLASS(max_message_size_exceeded, protocol::MAX_MESSAGE_SIZE_EXCEEDED, true);
            DEFINE_EXCEPTION_CLASS(wan_replication_queue_full, protocol::WAN_REPLICATION_QUEUE_FULL, true);
            DEFINE_EXCEPTION_CLASS(service_not_found, protocol::SERVICE_NOT_FOUND, true);

            DEFINE_EXCEPTION_CLASS(stale_task_id, protocol::STALE_TASK_ID, true);

            DEFINE_EXCEPTION_CLASS(duplicate_task, protocol::DUPLICATE_TASK, true);

            DEFINE_EXCEPTION_CLASS(stale_task, protocol::STALE_TASK, true);

            DEFINE_EXCEPTION_CLASS(local_member_reset, protocol::LOCAL_MEMBER_RESET, true);

            DEFINE_EXCEPTION_CLASS(indeterminate_operation_state, protocol::INDETERMINATE_OPERATION_STATE, true);

            DEFINE_EXCEPTION_CLASS(node_id_out_of_range, protocol::FLAKE_ID_NODE_ID_OUT_OF_RANGE_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(mutation_disallowed, protocol::MUTATION_DISALLOWED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(session_expired, protocol::SESSION_EXPIRED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(wait_key_cancelled , protocol::WAIT_KEY_CANCELLED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(lock_acquire_limit_reached, protocol::LOCK_ACQUIRE_LIMIT_REACHED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(lock_ownership_lost, protocol::LOCK_OWNERSHIP_LOST_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(cp_group_destroyed, protocol::CP_GROUP_DESTROYED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(cannot_replicate, protocol::CANNOT_REPLICATE_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(leader_demoted, protocol::LEADER_DEMOTED_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(stale_append_request, protocol::STALE_APPEND_REQUEST_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(not_leader, protocol::NOT_LEADER_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(version_mismatch, protocol::VERSION_MISMATCH_EXCEPTION, true);

            class HAZELCAST_API retryable_hazelcast : public hazelcast_ {
            public:
                retryable_hazelcast(const std::string &error_name, int32_t error_code, const std::string &source,
                                            const std::string &message, const std::string &details,
                                            std::exception_ptr cause, bool runtime,
                                            bool retryable);

                explicit retryable_hazelcast(const std::string &source = "", const std::string &message = "",
                                            const std::string &details = "", std::exception_ptr cause = nullptr);
            };

#define DEFINE_RETRYABLE_EXCEPTION_CLASS(ClassName, errorNo) \
            class HAZELCAST_API ClassName : public retryable_hazelcast {\
            public:\
                explicit ClassName(const std::string& source = "", const std::string& message = "", const std::string& details = "", std::exception_ptr cause = nullptr) \
                    : retryable_hazelcast(#ClassName, errorNo, source, message, details, cause, false, true) {} \
            };\

            /** List of Retryable exceptions **/
            DEFINE_RETRYABLE_EXCEPTION_CLASS(caller_not_member, protocol::CALLER_NOT_MEMBER);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(partition_migrating, protocol::PARTITION_MIGRATING);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(retryable_io, protocol::RETRYABLE_IO);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(target_not_member, protocol::TARGET_NOT_MEMBER);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(wrong_target, protocol::WRONG_TARGET);

            DEFINE_RETRYABLE_EXCEPTION_CLASS(target_not_replica, protocol::TARGET_NOT_REPLICA_EXCEPTION);

            class member_left : public execution {
            public:
                explicit member_left(const std::string &source = "", const std::string &message = "",
                                    const std::string &details = "", std::exception_ptr cause = nullptr);
            };

            // ---------  runtime ends here -------------------------------------------------/

            // -----------------    ONLY Client side exceptions start here ----------------------------------------
            // -----------------    Client side runtime exceptions start here --------------------------------
            class consistency_lost : public hazelcast_ {
            public:
                explicit consistency_lost(const std::string &source = "", const std::string &message = "",
                                         const std::string &details = "", std::exception_ptr cause = nullptr);
            };

            DEFINE_EXCEPTION_CLASS(hazelcast_client_not_active, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE, true);

            DEFINE_EXCEPTION_CLASS(hazelcast_client_offline, protocol::HAZELCAST_CLIENT_OFFLINE, true);

            // -----------------    Client side runtime exceptions finish here --------------------------------

            // -----------------    Client side non-runtime exceptions start here -----------------------------
            /**
             * Thrown when Hazelcast client is offline during an invocation.
             */
            DEFINE_EXCEPTION_CLASS(unknown_host, protocol::UNKNOWN_HOST, false);

            DEFINE_EXCEPTION_CLASS(client_not_allowed_in_cluster, protocol::CLIENT_NOT_ALLOWED_IN_CLUSTER, true);
            // -----------------    Client side non-runtime exceptions finish here ----------------------------

            // -----------------    Client side runtime exceptions ends here --------------------------------
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
