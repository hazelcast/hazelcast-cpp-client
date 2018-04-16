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
// Created by sancar koyunlu on 23/07/14.
//
#ifndef HAZELCAST_CLIENT_EXCEPTION_PROTOCOLEXCEPTIONS_H_
#define HAZELCAST_CLIENT_EXCEPTION_PROTOCOLEXCEPTIONS_H_

#include <stdint.h>

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
#define DEFINE_EXCEPTION_CLASS(ClassName, errorNo) \
            class HAZELCAST_API ClassName : public IException {\
            public:\
                static const int32_t ERROR_CODE = errorNo;\
                ClassName(const std::string& source, const std::string& message, const std::string& details, \
                        int32_t causeCode) \
                    : IException(#ClassName, source, message, details, ERROR_CODE, causeCode) {\
                }\
                ClassName(const std::string& source, const std::string& message, int32_t causeCode) \
                    : IException(#ClassName, source, message, ERROR_CODE, causeCode) {\
                }\
                ClassName(const std::string& source, const std::string& message) \
                    : IException(#ClassName, source, message, ERROR_CODE) {\
                }\
                ClassName(const std::string& source) : IException(#ClassName, source, "", ERROR_CODE) {\
                }\
                ClassName(const std::string &source, const std::string &message, \
                            const boost::shared_ptr<IException> &cause) \
                            : IException(#ClassName, source, message, ERROR_CODE, cause) {}\
                ClassName(const std::string &source, const std::string &message, const IException &cause) \
                            : IException(#ClassName, source, message, ERROR_CODE, boost::shared_ptr<IException>(cause.clone())) {}\
                virtual std::auto_ptr<IException> clone() const {\
                    return std::auto_ptr<IException>(new ClassName(*this));\
                } \
                void raise() const { throw *this; } \
            };\

            DEFINE_EXCEPTION_CLASS(ArrayIndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS);
            DEFINE_EXCEPTION_CLASS(ArrayStoreException, protocol::ARRAY_STORE);
            DEFINE_EXCEPTION_CLASS(AuthenticationException, protocol::AUTHENTICATIONERROR);
            DEFINE_EXCEPTION_CLASS(CacheNotExistsException, protocol::CACHE_NOT_EXISTS);
            DEFINE_EXCEPTION_CLASS(CallerNotMemberException, protocol::CALLER_NOT_MEMBER);
            DEFINE_EXCEPTION_CLASS(CancellationException, protocol::CANCELLATION);
            DEFINE_EXCEPTION_CLASS(ClassCastException, protocol::CLASS_CAST);
            DEFINE_EXCEPTION_CLASS(ClassNotFoundException, protocol::CLASS_NOT_FOUND);
            DEFINE_EXCEPTION_CLASS(ConcurrentModificationException, protocol::CONCURRENT_MODIFICATION);
            DEFINE_EXCEPTION_CLASS(ConfigMismatchException, protocol::CONFIG_MISMATCH);
            DEFINE_EXCEPTION_CLASS(ConfigurationException, protocol::CONFIGURATION);
            DEFINE_EXCEPTION_CLASS(DistributedObjectDestroyedException, protocol::DISTRIBUTED_OBJECT_DESTROYED);
            DEFINE_EXCEPTION_CLASS(DuplicateInstanceNameException, protocol::DUPLICATE_INSTANCE_NAME);
            DEFINE_EXCEPTION_CLASS(EOFException, protocol::ENDOFFILE);
            DEFINE_EXCEPTION_CLASS(ExecutionException, protocol::EXECUTION);
            DEFINE_EXCEPTION_CLASS(HazelcastException, protocol::HAZELCAST);
            DEFINE_EXCEPTION_CLASS(HazelcastInstanceNotActiveException, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE);
            DEFINE_EXCEPTION_CLASS(HazelcastOverloadException, protocol::HAZELCAST_OVERLOAD);
            DEFINE_EXCEPTION_CLASS(HazelcastSerializationException, protocol::HAZELCAST_SERIALIZATION);
            DEFINE_EXCEPTION_CLASS(IOException, protocol::IO);
            DEFINE_EXCEPTION_CLASS(IllegalArgumentException, protocol::ILLEGAL_ARGUMENT);
            DEFINE_EXCEPTION_CLASS(IllegalAccessException, protocol::ILLEGAL_ACCESS_EXCEPTION);
            DEFINE_EXCEPTION_CLASS(IllegalAccessError, protocol::ILLEGAL_ACCESS_ERROR);
            DEFINE_EXCEPTION_CLASS(IllegalMonitorStateException, protocol::ILLEGAL_MONITOR_STATE);
            DEFINE_EXCEPTION_CLASS(IllegalStateException, protocol::ILLEGAL_STATE);
            DEFINE_EXCEPTION_CLASS(IllegalThreadStateException, protocol::ILLEGAL_THREAD_STATE);
            DEFINE_EXCEPTION_CLASS(IndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS);
            DEFINE_EXCEPTION_CLASS(InterruptedException, protocol::INTERRUPTED);
            DEFINE_EXCEPTION_CLASS(InvalidAddressException, protocol::INVALID_ADDRESS);
            DEFINE_EXCEPTION_CLASS(InvalidConfigurationException, protocol::INVALID_CONFIGURATION);
            DEFINE_EXCEPTION_CLASS(MemberLeftException, protocol::MEMBER_LEFT);
            DEFINE_EXCEPTION_CLASS(NegativeArraySizeException, protocol::NEGATIVE_ARRAY_SIZE);
            DEFINE_EXCEPTION_CLASS(NoSuchElementException, protocol::NO_SUCH_ELEMENT);
            DEFINE_EXCEPTION_CLASS(NotSerializableException, protocol::NOT_SERIALIZABLE);
            DEFINE_EXCEPTION_CLASS(NullPointerException, protocol::NULL_POINTER);
            DEFINE_EXCEPTION_CLASS(OperationTimeoutException, protocol::OPERATION_TIMEOUT);
            DEFINE_EXCEPTION_CLASS(PartitionMigratingException, protocol::PARTITION_MIGRATING);
            DEFINE_EXCEPTION_CLASS(QueryException, protocol::QUERY);
            DEFINE_EXCEPTION_CLASS(QueryResultSizeExceededException, protocol::QUERY_RESULT_SIZE_EXCEEDED);
            DEFINE_EXCEPTION_CLASS(QuorumException, protocol::QUORUM);
            DEFINE_EXCEPTION_CLASS(ReachedMaxSizeException, protocol::REACHED_MAX_SIZE);
            DEFINE_EXCEPTION_CLASS(RejectedExecutionException, protocol::REJECTED_EXECUTION);
            DEFINE_EXCEPTION_CLASS(RemoteMapReduceException, protocol::REMOTE_MAP_REDUCE);
            DEFINE_EXCEPTION_CLASS(ResponseAlreadySentException, protocol::RESPONSE_ALREADY_SENT);
            DEFINE_EXCEPTION_CLASS(RetryableHazelcastException, protocol::RETRYABLE_HAZELCAST);
            DEFINE_EXCEPTION_CLASS(RetryableIOException, protocol::RETRYABLE_IO);
            DEFINE_EXCEPTION_CLASS(RuntimeException, protocol::RUNTIME);
            DEFINE_EXCEPTION_CLASS(SecurityException, protocol::SECURITY);
            DEFINE_EXCEPTION_CLASS(SocketException, protocol::SOCKET);
            /**
            * Raised at Ringbuffer::readOne if the provided sequence is smaller then Ringbuffer::headSequence() is passed.
            */
            DEFINE_EXCEPTION_CLASS(StaleSequenceException, protocol::STALE_SEQUENCE);
            DEFINE_EXCEPTION_CLASS(TargetDisconnectedException, protocol::TARGET_DISCONNECTED);
            DEFINE_EXCEPTION_CLASS(TargetNotMemberException, protocol::TARGET_NOT_MEMBER);
            DEFINE_EXCEPTION_CLASS(TimeoutException, protocol::TIMEOUT);
            DEFINE_EXCEPTION_CLASS(TopicOverloadException, protocol::TOPIC_OVERLOAD);
            DEFINE_EXCEPTION_CLASS(TopologyChangedException, protocol::TOPOLOGY_CHANGED);
            DEFINE_EXCEPTION_CLASS(TransactionException, protocol::TRANSACTION);
            DEFINE_EXCEPTION_CLASS(TransactionNotActiveException, protocol::TRANSACTION_NOT_ACTIVE);
            DEFINE_EXCEPTION_CLASS(TransactionTimedOutException, protocol::TRANSACTION_TIMED_OUT);
            DEFINE_EXCEPTION_CLASS(URISyntaxException, protocol::URI_SYNTAX);
            DEFINE_EXCEPTION_CLASS(UTFDataFormatException, protocol::UTF_DATA_FORMAT);
            DEFINE_EXCEPTION_CLASS(UnsupportedOperationException, protocol::UNSUPPORTED_OPERATION);
            DEFINE_EXCEPTION_CLASS(WrongTargetException, protocol::WRONG_TARGET);
            DEFINE_EXCEPTION_CLASS(XAException, protocol::XA);
            DEFINE_EXCEPTION_CLASS(AccessControlException, protocol::ACCESS_CONTROL);
            DEFINE_EXCEPTION_CLASS(LoginException, protocol::LOGIN);
            DEFINE_EXCEPTION_CLASS(UnsupportedCallbackException, protocol::UNSUPPORTED_CALLBACK);
            DEFINE_EXCEPTION_CLASS(NoDataMemberInClusterException, protocol::NO_DATA_MEMBER);
            DEFINE_EXCEPTION_CLASS(ReplicatedMapCantBeCreatedOnLiteMemberException, protocol::REPLICATED_MAP_CANT_BE_CREATED);
            DEFINE_EXCEPTION_CLASS(MaxMessageSizeExceeded, protocol::MAX_MESSAGE_SIZE_EXCEEDED);
            DEFINE_EXCEPTION_CLASS(WANReplicationQueueFullException, protocol::WAN_REPLICATION_QUEUE_FULL);
            DEFINE_EXCEPTION_CLASS(AssertionError, protocol::ASSERTION_ERROR);
            DEFINE_EXCEPTION_CLASS(OutOfMemoryError, protocol::OUT_OF_MEMORY_ERROR);
            DEFINE_EXCEPTION_CLASS(StackOverflowError, protocol::STACK_OVERFLOW_ERROR);
            DEFINE_EXCEPTION_CLASS(NativeOutOfMemoryError, protocol::NATIVE_OUT_OF_MEMORY_ERROR);
            DEFINE_EXCEPTION_CLASS(ServiceNotFoundException, protocol::SERVICE_NOT_FOUND);

            // -----------------    ONLY Client side exceptions below -------------------------------------------
            DEFINE_EXCEPTION_CLASS(HazelcastClientNotActiveException, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE);
            /**
             * Thrown when Hazelcast client is offline during an invocation.
             */
            DEFINE_EXCEPTION_CLASS(HazelcastClientOfflineException, protocol::HAZELCAST_CLIENT_OFFLINE);
            DEFINE_EXCEPTION_CLASS(UnknownHostException, protocol::UNKNOWN_HOST);
            DEFINE_EXCEPTION_CLASS(FutureUninitialized, protocol::FUTURE_UNINITIALIZED);

            class HAZELCAST_API UndefinedErrorCodeException : public IException {
            public:
                UndefinedErrorCodeException(const std::string &source, const std::string &message,
                                                            int32_t errorCode, int64_t correlationId,
                                                            std::string details);

                virtual ~UndefinedErrorCodeException() throw();

                int32_t getUndefinedErrorCode() const;

                int64_t getMessageCallId() const;

                const std::string &getDetailedErrorMessage() const;

                virtual std::auto_ptr<IException> clone() const;

            private:
                int32_t error;
                int64_t messageCallId;
                std::string detailedErrorMessage;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_EXCEPTION_PROTOCOLEXCEPTIONS_H_
