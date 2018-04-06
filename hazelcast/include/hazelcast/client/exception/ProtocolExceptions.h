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
            class HAZELCAST_API ProtocolException : public IException {
            public:
                ProtocolException(const std::string &source, const std::string &message, const std::string &details,
                                  int32_t errorNo, int32_t causeCode);

                ProtocolException(const std::string &source, const std::string &message, int32_t errorNo,
                                  int32_t causeCode);

                ProtocolException(const std::string &source, const std::string &message, int32_t errorNo);

                ProtocolException(const std::string &source, const std::string &message, int32_t errorNo,
                                  const boost::shared_ptr<IException> &cause);

                int32_t getErrorCode() const;

                int32_t getCauseErrorCode() const;

                virtual std::auto_ptr<IException> clone() const;

                void raise() const;

                virtual bool isProtocolException() const;

            private:
                int32_t errorCode;
                int32_t causeErrorCode;
            };

#define DEFINE_PROTOCOL_EXCEPTION(ClassName, errorNo) \
            class HAZELCAST_API ClassName : public ProtocolException {\
            public:\
                static const int32_t ERROR_CODE = errorNo;\
                ClassName(const std::string& source, const std::string& message, const std::string& details, \
                        int32_t causeCode) \
                    : ProtocolException(source, message, details, ERROR_CODE, causeCode) {\
                }\
                ClassName(const std::string& source, const std::string& message, int32_t causeCode) \
                    : ProtocolException(source, message, ERROR_CODE, causeCode) {\
                }\
                ClassName(const std::string& source, const std::string& message) \
                    : ProtocolException(source, message, ERROR_CODE) {\
                }\
                ClassName(const std::string& source) : ProtocolException(source, "", ERROR_CODE) {\
                }\
                ClassName(const std::string &source, const std::string &message, \
                            const boost::shared_ptr<IException> &cause) : ProtocolException(source, message, ERROR_CODE, cause) {}\
                virtual std::auto_ptr<IException> clone() const {\
                    return std::auto_ptr<IException>(new ClassName(*this));\
                } \
                void raise() const { throw *this; } \
            }\

            DEFINE_PROTOCOL_EXCEPTION(ArrayIndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS);
            DEFINE_PROTOCOL_EXCEPTION(ArrayStoreException, protocol::ARRAY_STORE);
            DEFINE_PROTOCOL_EXCEPTION(AuthenticationException, protocol::AUTHENTICATIONERROR);
            DEFINE_PROTOCOL_EXCEPTION(CacheNotExistsException, protocol::CACHE_NOT_EXISTS);
            DEFINE_PROTOCOL_EXCEPTION(CallerNotMemberException, protocol::CALLER_NOT_MEMBER);
            DEFINE_PROTOCOL_EXCEPTION(CancellationException, protocol::CANCELLATION);
            DEFINE_PROTOCOL_EXCEPTION(ClassCastException, protocol::CLASS_CAST);
            DEFINE_PROTOCOL_EXCEPTION(ClassNotFoundException, protocol::CLASS_NOT_FOUND);
            DEFINE_PROTOCOL_EXCEPTION(ConcurrentModificationException, protocol::CONCURRENT_MODIFICATION);
            DEFINE_PROTOCOL_EXCEPTION(ConfigMismatchException, protocol::CONFIG_MISMATCH);
            DEFINE_PROTOCOL_EXCEPTION(ConfigurationException, protocol::CONFIGURATION);
            DEFINE_PROTOCOL_EXCEPTION(DistributedObjectDestroyedException, protocol::DISTRIBUTED_OBJECT_DESTROYED);
            DEFINE_PROTOCOL_EXCEPTION(DuplicateInstanceNameException, protocol::DUPLICATE_INSTANCE_NAME);
            DEFINE_PROTOCOL_EXCEPTION(EOFException, protocol::ENDOFFILE);
            DEFINE_PROTOCOL_EXCEPTION(ExecutionException, protocol::EXECUTION);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastException, protocol::HAZELCAST);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastInstanceNotActiveException, protocol::HAZELCAST_INSTANCE_NOT_ACTIVE);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastOverloadException, protocol::HAZELCAST_OVERLOAD);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastSerializationException, protocol::HAZELCAST_SERIALIZATION);
            DEFINE_PROTOCOL_EXCEPTION(IOException, protocol::IO);
            DEFINE_PROTOCOL_EXCEPTION(IllegalArgumentException, protocol::ILLEGAL_ARGUMENT);
            DEFINE_PROTOCOL_EXCEPTION(IllegalAccessException, protocol::ILLEGAL_ACCESS_EXCEPTION);
            DEFINE_PROTOCOL_EXCEPTION(IllegalAccessError, protocol::ILLEGAL_ACCESS_ERROR);
            DEFINE_PROTOCOL_EXCEPTION(IllegalMonitorStateException, protocol::ILLEGAL_MONITOR_STATE);
            DEFINE_PROTOCOL_EXCEPTION(IllegalStateException, protocol::ILLEGAL_STATE);
            DEFINE_PROTOCOL_EXCEPTION(IllegalThreadStateException, protocol::ILLEGAL_THREAD_STATE);
            DEFINE_PROTOCOL_EXCEPTION(IndexOutOfBoundsException, protocol::INDEX_OUT_OF_BOUNDS);
            DEFINE_PROTOCOL_EXCEPTION(InterruptedException, protocol::INTERRUPTED);
            DEFINE_PROTOCOL_EXCEPTION(InvalidAddressException, protocol::INVALID_ADDRESS);
            DEFINE_PROTOCOL_EXCEPTION(InvalidConfigurationException, protocol::INVALID_CONFIGURATION);
            DEFINE_PROTOCOL_EXCEPTION(MemberLeftException, protocol::MEMBER_LEFT);
            DEFINE_PROTOCOL_EXCEPTION(NegativeArraySizeException, protocol::NEGATIVE_ARRAY_SIZE);
            DEFINE_PROTOCOL_EXCEPTION(NoSuchElementException, protocol::NO_SUCH_ELEMENT);
            DEFINE_PROTOCOL_EXCEPTION(NotSerializableException, protocol::NOT_SERIALIZABLE);
            DEFINE_PROTOCOL_EXCEPTION(NullPointerException, protocol::NULL_POINTER);
            DEFINE_PROTOCOL_EXCEPTION(OperationTimeoutException, protocol::OPERATION_TIMEOUT);
            DEFINE_PROTOCOL_EXCEPTION(PartitionMigratingException, protocol::PARTITION_MIGRATING);
            DEFINE_PROTOCOL_EXCEPTION(QueryException, protocol::QUERY);
            DEFINE_PROTOCOL_EXCEPTION(QueryResultSizeExceededException, protocol::QUERY_RESULT_SIZE_EXCEEDED);
            DEFINE_PROTOCOL_EXCEPTION(QuorumException, protocol::QUORUM);
            DEFINE_PROTOCOL_EXCEPTION(ReachedMaxSizeException, protocol::REACHED_MAX_SIZE);
            DEFINE_PROTOCOL_EXCEPTION(RejectedExecutionException, protocol::REJECTED_EXECUTION);
            DEFINE_PROTOCOL_EXCEPTION(RemoteMapReduceException, protocol::REMOTE_MAP_REDUCE);
            DEFINE_PROTOCOL_EXCEPTION(ResponseAlreadySentException, protocol::RESPONSE_ALREADY_SENT);
            DEFINE_PROTOCOL_EXCEPTION(RetryableHazelcastException, protocol::RETRYABLE_HAZELCAST);
            DEFINE_PROTOCOL_EXCEPTION(RetryableIOException, protocol::RETRYABLE_IO);
            DEFINE_PROTOCOL_EXCEPTION(RuntimeException, protocol::RUNTIME);
            DEFINE_PROTOCOL_EXCEPTION(SecurityException, protocol::SECURITY);
            DEFINE_PROTOCOL_EXCEPTION(SocketException, protocol::SOCKET);
            /**
            * Raised at Ringbuffer::readOne if the provided sequence is smaller then Ringbuffer::headSequence() is passed.
            */
            DEFINE_PROTOCOL_EXCEPTION(StaleSequenceException, protocol::STALE_SEQUENCE);
            DEFINE_PROTOCOL_EXCEPTION(TargetDisconnectedException, protocol::TARGET_DISCONNECTED);
            DEFINE_PROTOCOL_EXCEPTION(TargetNotMemberException, protocol::TARGET_NOT_MEMBER);
            DEFINE_PROTOCOL_EXCEPTION(TimeoutException, protocol::TIMEOUT);
            DEFINE_PROTOCOL_EXCEPTION(TopicOverloadException, protocol::TOPIC_OVERLOAD);
            DEFINE_PROTOCOL_EXCEPTION(TopologyChangedException, protocol::TOPOLOGY_CHANGED);
            DEFINE_PROTOCOL_EXCEPTION(TransactionException, protocol::TRANSACTION);
            DEFINE_PROTOCOL_EXCEPTION(TransactionNotActiveException, protocol::TRANSACTION_NOT_ACTIVE);
            DEFINE_PROTOCOL_EXCEPTION(TransactionTimedOutException, protocol::TRANSACTION_TIMED_OUT);
            DEFINE_PROTOCOL_EXCEPTION(URISyntaxException, protocol::URI_SYNTAX);
            DEFINE_PROTOCOL_EXCEPTION(UTFDataFormatException, protocol::UTF_DATA_FORMAT);
            DEFINE_PROTOCOL_EXCEPTION(UnsupportedOperationException, protocol::UNSUPPORTED_OPERATION);
            DEFINE_PROTOCOL_EXCEPTION(WrongTargetException, protocol::WRONG_TARGET);
            DEFINE_PROTOCOL_EXCEPTION(XAException, protocol::XA);
            DEFINE_PROTOCOL_EXCEPTION(AccessControlException, protocol::ACCESS_CONTROL);
            DEFINE_PROTOCOL_EXCEPTION(LoginException, protocol::LOGIN);
            DEFINE_PROTOCOL_EXCEPTION(UnsupportedCallbackException, protocol::UNSUPPORTED_CALLBACK);
            DEFINE_PROTOCOL_EXCEPTION(NoDataMemberInClusterException, protocol::NO_DATA_MEMBER);
            DEFINE_PROTOCOL_EXCEPTION(ReplicatedMapCantBeCreatedOnLiteMemberException, protocol::REPLICATED_MAP_CANT_BE_CREATED);
            DEFINE_PROTOCOL_EXCEPTION(MaxMessageSizeExceeded, protocol::MAX_MESSAGE_SIZE_EXCEEDED);
            DEFINE_PROTOCOL_EXCEPTION(WANReplicationQueueFullException, protocol::WAN_REPLICATION_QUEUE_FULL);
            DEFINE_PROTOCOL_EXCEPTION(AssertionError, protocol::ASSERTION_ERROR);
            DEFINE_PROTOCOL_EXCEPTION(OutOfMemoryError, protocol::OUT_OF_MEMORY_ERROR);
            DEFINE_PROTOCOL_EXCEPTION(StackOverflowError, protocol::STACK_OVERFLOW_ERROR);
            DEFINE_PROTOCOL_EXCEPTION(NativeOutOfMemoryError, protocol::NATIVE_OUT_OF_MEMORY_ERROR);
            DEFINE_PROTOCOL_EXCEPTION(ServiceNotFoundException, protocol::SERVICE_NOT_FOUND);

            class HAZELCAST_API UndefinedErrorCodeException : public IException {
            public:
                UndefinedErrorCodeException(int32_t errorCode, int64_t correlationId, std::string details);

                virtual ~UndefinedErrorCodeException() throw();

                int32_t getErrorCode() const;

                int64_t getMessageCallId() const;

                const std::string &getDetailedErrorMessage() const;

            private:
                int32_t error;
                int64_t messageCallId;
                std::string detailedErrorMessage;
            };

            class HAZELCAST_API HazelcastClientNotActiveException : public IException {
            public:
                HazelcastClientNotActiveException(const std::string &source, const std::string &message);

                virtual ~HazelcastClientNotActiveException() throw();
            };

            /**
             * Thrown when Hazelcast client is offline during an invocation.
             */
            class HAZELCAST_API HazelcastClientOfflineException : public IllegalStateException {
            public:
                HazelcastClientOfflineException(const std::string &source, const std::string &message);

                virtual ~HazelcastClientOfflineException() throw();
            };

            class HAZELCAST_API UnknownHostException : public IException {
            public:
                UnknownHostException(const std::string &source, const std::string &message);
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_EXCEPTION_PROTOCOLEXCEPTIONS_H_
