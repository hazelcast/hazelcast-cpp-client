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

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
#define DEFINE_EXCEPTION_CLASS(ClassName, errorNo, isRuntime) \
            class HAZELCAST_API ClassName : public IException {\
            public:\
                ClassName(const std::string& source, const std::string& message, \
                        const std::string& details = "", std::exception_ptr cause = nullptr, bool retryable = false) \
                    : ClassName(#ClassName, errorNo, source, message, details, cause, isRuntime, retryable) {}\
                explicit ClassName(const std::string& message) : ClassName("", message) {}\
                ClassName() : ClassName("", "") {}\
                ClassName(const std::string& errorName, int32_t errorCode, const std::string& source, const std::string& message, \
                        const std::string& details, std::exception_ptr cause, bool runtime, bool retryable) \
                    : IException(errorName, source, message, details, errorCode, cause, runtime, retryable) {}\
            };\

            // ---------  Non-RuntimeException starts here -------------------------------------------/
            DEFINE_EXCEPTION_CLASS(UndefinedErrorCodeException, protocol::UNDEFINED, false);

            DEFINE_EXCEPTION_CLASS(ExecutionException, protocol::EXECUTION, false);

            DEFINE_EXCEPTION_CLASS(ClassNotFoundException, protocol::CLASS_NOT_FOUND, false);

            DEFINE_EXCEPTION_CLASS(EOFException, protocol::ENDOFFILE, false);

            DEFINE_EXCEPTION_CLASS(IOException, protocol::IO, false);

            DEFINE_EXCEPTION_CLASS(IllegalAccessException, protocol::ILLEGAL_ACCESS_EXCEPTION, false);

            DEFINE_EXCEPTION_CLASS(IllegalAccessError, protocol::ILLEGAL_ACCESS_ERROR, false);

            DEFINE_EXCEPTION_CLASS(InterruptedException, protocol::INTERRUPTED, false);

            DEFINE_EXCEPTION_CLASS(NotSerializableException, protocol::NOT_SERIALIZABLE, false);

            DEFINE_EXCEPTION_CLASS(SocketException, protocol::SOCK_ERROR, false);

            DEFINE_EXCEPTION_CLASS(TimeoutException, protocol::TIMEOUT, false);
            DEFINE_EXCEPTION_CLASS(URISyntaxException, protocol::URI_SYNTAX, false);
            DEFINE_EXCEPTION_CLASS(UTFDataFormatException, protocol::UTF_DATA_FORMAT, false);
            DEFINE_EXCEPTION_CLASS(XAException, protocol::XA, false);
            DEFINE_EXCEPTION_CLASS(LoginException, protocol::LOGIN, false);
            DEFINE_EXCEPTION_CLASS(UnsupportedCallbackException, protocol::UNSUPPORTED_CALLBACK, false);
            DEFINE_EXCEPTION_CLASS(AssertionError, protocol::ASSERTION_ERROR, false);
            DEFINE_EXCEPTION_CLASS(OutOfMemoryError, protocol::OUT_OF_MEMORY_ERROR, false);
            DEFINE_EXCEPTION_CLASS(StackOverflowError, protocol::STACK_OVERFLOW_ERROR, false);
            DEFINE_EXCEPTION_CLASS(NativeOutOfMemoryError, protocol::NATIVE_OUT_OF_MEMORY_ERROR, false);
            // ---------  Non-RuntimeException ends here -------------------------------------------/

            // ---------  RuntimeException starts here -------------------------------------------/
            DEFINE_EXCEPTION_CLASS(ArrayIndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS, true);
            DEFINE_EXCEPTION_CLASS(ArrayStoreException, protocol::ARRAY_STORE, true);
            DEFINE_EXCEPTION_CLASS(AuthenticationException, protocol::AUTHENTICATION, true);
            DEFINE_EXCEPTION_CLASS(CacheNotExistsException, protocol::CACHE_NOT_EXISTS, true);
            DEFINE_EXCEPTION_CLASS(CancellationException, protocol::CANCELLATION, true);
            DEFINE_EXCEPTION_CLASS(ClassCastException, protocol::CLASS_CAST, true);
            DEFINE_EXCEPTION_CLASS(ConcurrentModificationException, protocol::CONCURRENT_MODIFICATION, true);
            DEFINE_EXCEPTION_CLASS(ConfigMismatchException, protocol::CONFIG_MISMATCH, true);
            DEFINE_EXCEPTION_CLASS(DistributedObjectDestroyedException, protocol::DISTRIBUTED_OBJECT_DESTROYED, true);
            DEFINE_EXCEPTION_CLASS(EntryProcessorException, protocol::ENTRY_PROCESSOR, true);
            DEFINE_EXCEPTION_CLASS(HazelcastException, protocol::HAZELCAST, true);
            DEFINE_EXCEPTION_CLASS(HazelcastInstanceNotActiveException, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE, true);
            DEFINE_EXCEPTION_CLASS(HazelcastOverloadException, protocol::HAZELCAST_OVERLOAD, true);
            DEFINE_EXCEPTION_CLASS(HazelcastSerializationException, protocol::HAZELCAST_SERIALIZATION, true);
            DEFINE_EXCEPTION_CLASS(IllegalArgumentException, protocol::ILLEGAL_ARGUMENT, true);
            DEFINE_EXCEPTION_CLASS(IllegalMonitorStateException, protocol::ILLEGAL_MONITOR_STATE, true);
            DEFINE_EXCEPTION_CLASS(IllegalStateException, protocol::ILLEGAL_STATE, true);
            DEFINE_EXCEPTION_CLASS(IllegalThreadStateException, protocol::ILLEGAL_THREAD_STATE, true);
            DEFINE_EXCEPTION_CLASS(IndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS, true);
            DEFINE_EXCEPTION_CLASS(InvalidAddressException, protocol::INVALID_ADDRESS, true);
            DEFINE_EXCEPTION_CLASS(InvalidConfigurationException, protocol::INVALID_CONFIGURATION, true);
            DEFINE_EXCEPTION_CLASS(NegativeArraySizeException, protocol::NEGATIVE_ARRAY_SIZE, true);
            DEFINE_EXCEPTION_CLASS(NoSuchElementException, protocol::NO_SUCH_ELEMENT, true);
            DEFINE_EXCEPTION_CLASS(NullPointerException, protocol::NULL_POINTER, true);
            DEFINE_EXCEPTION_CLASS(OperationTimeoutException, protocol::OPERATION_TIMEOUT, true);
            DEFINE_EXCEPTION_CLASS(QueryException, protocol::QUERY, true);
            DEFINE_EXCEPTION_CLASS(QueryResultSizeExceededException, protocol::QUERY_RESULT_SIZE_EXCEEDED, true);
            DEFINE_EXCEPTION_CLASS(SplitBrainProtectionException, protocol::SPLIT_BRAIN_PROTECTION, true);
            DEFINE_EXCEPTION_CLASS(ReachedMaxSizeException, protocol::REACHED_MAX_SIZE, true);
            DEFINE_EXCEPTION_CLASS(RejectedExecutionException, protocol::REJECTED_EXECUTION, true);
            DEFINE_EXCEPTION_CLASS(ResponseAlreadySentException, protocol::RESPONSE_ALREADY_SENT, true);
            DEFINE_EXCEPTION_CLASS(RuntimeException, protocol::RUNTIME, true);
            DEFINE_EXCEPTION_CLASS(SecurityException, protocol::SECURITY, true);
            /**
            * Raised at Ringbuffer::readOne if the provided sequence is smaller then Ringbuffer::headSequence() is passed.
            */
            DEFINE_EXCEPTION_CLASS(StaleSequenceException, protocol::STALE_SEQUENCE, true);
            DEFINE_EXCEPTION_CLASS(TargetDisconnectedException, protocol::TARGET_DISCONNECTED, true);
            DEFINE_EXCEPTION_CLASS(TopicOverloadException, protocol::TOPIC_OVERLOAD, true);

            DEFINE_EXCEPTION_CLASS(TransactionException, protocol::TRANSACTION, true);
            DEFINE_EXCEPTION_CLASS(TransactionNotActiveException, protocol::TRANSACTION_NOT_ACTIVE, true);
            DEFINE_EXCEPTION_CLASS(TransactionTimedOutException, protocol::TRANSACTION_TIMED_OUT, true);
            DEFINE_EXCEPTION_CLASS(UnsupportedOperationException, protocol::UNSUPPORTED_OPERATION, true);
            DEFINE_EXCEPTION_CLASS(AccessControlException, protocol::ACCESS_CONTROL, true);
            DEFINE_EXCEPTION_CLASS(NoDataMemberInClusterException, protocol::NO_DATA_MEMBER, true);
            DEFINE_EXCEPTION_CLASS(ReplicatedMapCantBeCreatedOnLiteMemberException, protocol::REPLICATED_MAP_CANT_BE_CREATED, true);
            DEFINE_EXCEPTION_CLASS(MaxMessageSizeExceeded, protocol::MAX_MESSAGE_SIZE_EXCEEDED, true);
            DEFINE_EXCEPTION_CLASS(WANReplicationQueueFullException, protocol::WAN_REPLICATION_QUEUE_FULL, true);
            DEFINE_EXCEPTION_CLASS(ServiceNotFoundException, protocol::SERVICE_NOT_FOUND, true);

            DEFINE_EXCEPTION_CLASS(StaleTaskIdException, protocol::STALE_TASK_ID, true);

            DEFINE_EXCEPTION_CLASS(DuplicateTaskException, protocol::DUPLICATE_TASK, true);

            DEFINE_EXCEPTION_CLASS(StaleTaskException, protocol::STALE_TASK, true);

            DEFINE_EXCEPTION_CLASS(LocalMemberResetException, protocol::LOCAL_MEMBER_RESET, true);

            DEFINE_EXCEPTION_CLASS(IndeterminateOperationStateException, protocol::INDETERMINATE_OPERATION_STATE, true);

            DEFINE_EXCEPTION_CLASS(NodeIdOutOfRangeException, protocol::FLAKE_ID_NODE_ID_OUT_OF_RANGE_EXCEPTION, true);

            DEFINE_EXCEPTION_CLASS(MutationDisallowedException, protocol::MUTATION_DISALLOWED_EXCEPTION, true);

            class HAZELCAST_API RetryableHazelcastException : public HazelcastException {
            public:
                RetryableHazelcastException(const std::string &errorName, int32_t errorCode, const std::string &source,
                                            const std::string &message, const std::string &details,
                                            std::exception_ptr cause, bool runtime,
                                            bool retryable);

                explicit RetryableHazelcastException(const std::string &source = "", const std::string &message = "",
                                            const std::string &details = "", std::exception_ptr cause = nullptr);
            };

#define DEFINE_RETRYABLE_EXCEPTION_CLASS(ClassName, errorNo) \
            class HAZELCAST_API ClassName : public RetryableHazelcastException {\
            public:\
                explicit ClassName(const std::string& source = "", const std::string& message = "", const std::string& details = "", std::exception_ptr cause = nullptr) \
                    : RetryableHazelcastException(#ClassName, errorNo, source, message, details, cause, false, true) {} \
            };\

            /** List of Retryable exceptions **/
            DEFINE_RETRYABLE_EXCEPTION_CLASS(CallerNotMemberException, protocol::CALLER_NOT_MEMBER);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(PartitionMigratingException, protocol::PARTITION_MIGRATING);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(RetryableIOException, protocol::RETRYABLE_IO);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(TargetNotMemberException, protocol::TARGET_NOT_MEMBER);
            DEFINE_RETRYABLE_EXCEPTION_CLASS(WrongTargetException, protocol::WRONG_TARGET);

            DEFINE_RETRYABLE_EXCEPTION_CLASS(TargetNotReplicaException, protocol::TARGET_NOT_REPLICA_EXCEPTION);

            class MemberLeftException : public ExecutionException {
            public:
                explicit MemberLeftException(const std::string &source = "", const std::string &message = "",
                                    const std::string &details = "", std::exception_ptr cause = nullptr);
            };

            // ---------  RuntimeException ends here -------------------------------------------------/

            // -----------------    ONLY Client side exceptions start here ----------------------------------------
            // -----------------    Client side runtime exceptions start here --------------------------------
            class ConsistencyLostException : public HazelcastException {
            public:
                explicit ConsistencyLostException(const std::string &source = "", const std::string &message = "",
                                         const std::string &details = "", std::exception_ptr cause = nullptr);
            };

            DEFINE_EXCEPTION_CLASS(HazelcastClientNotActiveException, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE, true);

            DEFINE_EXCEPTION_CLASS(HazelcastClientOfflineException, protocol::HAZELCAST_CLIENT_OFFLINE, true);

            // -----------------    Client side runtime exceptions finish here --------------------------------

            // -----------------    Client side non-runtime exceptions start here -----------------------------
            /**
             * Thrown when Hazelcast client is offline during an invocation.
             */
            DEFINE_EXCEPTION_CLASS(UnknownHostException, protocol::UNKNOWN_HOST, false);

            DEFINE_EXCEPTION_CLASS(ClientNotAllowedInClusterException, protocol::CLIENT_NOT_ALLOWED_IN_CLUSTER, true);
            // -----------------    Client side non-runtime exceptions finish here ----------------------------

            // -----------------    Client side runtime exceptions ends here --------------------------------
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
