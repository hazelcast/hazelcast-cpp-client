/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API ProtocolException : public IException {
            public:
                ProtocolException(const std::string& message, const std::string& details, int32_t errorNo,
                                  int32_t causeCode)
                        : IException("Cluster", details), errorCode(errorNo), causeErrorCode(causeCode) {
                }

                ProtocolException(const std::string& source, const std::string& message)
                        : IException(source, message), errorCode(-1), causeErrorCode(-1) {
                }

                int32_t getErrorCode() const {
                    return errorCode;
                }

                int32_t getCauseErrorCode() const {
                    return causeErrorCode;
                }
            private:
                int32_t errorCode;
                int32_t causeErrorCode;
            };

#define DEFINE_PROTOCOL_EXCEPTION(ClassName) \
            class HAZELCAST_API ClassName : public ProtocolException {\
            public:\
                ClassName(const std::string& message, const std::string& details, int32_t errorNo, int32_t causeCode) \
                    : ProtocolException(message, details, errorNo, causeCode) {\
                }\
                ClassName(const std::string& source, const std::string& message) \
                    : ProtocolException(source, message) {\
                }\
                ClassName(const std::string& source) : ProtocolException(source, "") {\
                }\
                virtual void raise() {\
                    throw *this;\
                }\
            }\

            DEFINE_PROTOCOL_EXCEPTION(ArrayIndexOutOfBoundsException);
            DEFINE_PROTOCOL_EXCEPTION(ArrayStoreException);
            DEFINE_PROTOCOL_EXCEPTION(AuthenticationException);
            DEFINE_PROTOCOL_EXCEPTION(CacheNotExistsException);
            DEFINE_PROTOCOL_EXCEPTION(CallerNotMemberException);
            DEFINE_PROTOCOL_EXCEPTION(CancellationException);
            DEFINE_PROTOCOL_EXCEPTION(ClassCastException);
            DEFINE_PROTOCOL_EXCEPTION(ClassNotFoundException);
            DEFINE_PROTOCOL_EXCEPTION(ConcurrentModificationException);
            DEFINE_PROTOCOL_EXCEPTION(ConfigMismatchException);
            DEFINE_PROTOCOL_EXCEPTION(ConfigurationException);
            DEFINE_PROTOCOL_EXCEPTION(DistributedObjectDestroyedException);
            DEFINE_PROTOCOL_EXCEPTION(DuplicateInstanceNameException);
            DEFINE_PROTOCOL_EXCEPTION(EOFException);
            DEFINE_PROTOCOL_EXCEPTION(ExecutionException);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastException);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastInstanceNotActiveException);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastOverloadException);
            DEFINE_PROTOCOL_EXCEPTION(HazelcastSerializationException);
            DEFINE_PROTOCOL_EXCEPTION(IOException);
            DEFINE_PROTOCOL_EXCEPTION(IllegalArgumentException);
            DEFINE_PROTOCOL_EXCEPTION(IllegalAccessException);
            DEFINE_PROTOCOL_EXCEPTION(IllegalAccessError);
            DEFINE_PROTOCOL_EXCEPTION(IllegalMonitorStateException);
            DEFINE_PROTOCOL_EXCEPTION(IllegalStateException);
            DEFINE_PROTOCOL_EXCEPTION(IllegalThreadStateException);
            DEFINE_PROTOCOL_EXCEPTION(IndexOutOfBoundsException);
            DEFINE_PROTOCOL_EXCEPTION(InterruptedException);
            DEFINE_PROTOCOL_EXCEPTION(InvalidAddressException);
            DEFINE_PROTOCOL_EXCEPTION(InvalidConfigurationException);
            DEFINE_PROTOCOL_EXCEPTION(MemberLeftException);
            DEFINE_PROTOCOL_EXCEPTION(NegativeArraySizeException);
            DEFINE_PROTOCOL_EXCEPTION(NoSuchElementException);
            DEFINE_PROTOCOL_EXCEPTION(NotSerializableException);
            DEFINE_PROTOCOL_EXCEPTION(NullPointerException);
            DEFINE_PROTOCOL_EXCEPTION(OperationTimeoutException);
            DEFINE_PROTOCOL_EXCEPTION(PartitionMigratingException);
            DEFINE_PROTOCOL_EXCEPTION(QueryException);
            DEFINE_PROTOCOL_EXCEPTION(QueryResultSizeExceededException);
            DEFINE_PROTOCOL_EXCEPTION(QuorumException);
            DEFINE_PROTOCOL_EXCEPTION(ReachedMaxSizeException);
            DEFINE_PROTOCOL_EXCEPTION(RejectedExecutionException);
            DEFINE_PROTOCOL_EXCEPTION(RemoteMapReduceException);
            DEFINE_PROTOCOL_EXCEPTION(ResponseAlreadySentException);
            DEFINE_PROTOCOL_EXCEPTION(RetryableHazelcastException);
            DEFINE_PROTOCOL_EXCEPTION(RetryableIOException);
            DEFINE_PROTOCOL_EXCEPTION(RuntimeException);
            DEFINE_PROTOCOL_EXCEPTION(SecurityException);
            DEFINE_PROTOCOL_EXCEPTION(SocketException);
            /**
            * Raised at Ringbuffer::readOne if the provided sequence is smaller then Ringbuffer::headSequence() is passed.
            */
            DEFINE_PROTOCOL_EXCEPTION(StaleSequenceException);
            DEFINE_PROTOCOL_EXCEPTION(TargetDisconnectedException);
            DEFINE_PROTOCOL_EXCEPTION(TargetNotMemberException);
            DEFINE_PROTOCOL_EXCEPTION(TimeoutException);
            DEFINE_PROTOCOL_EXCEPTION(TopicOverloadException);
            DEFINE_PROTOCOL_EXCEPTION(TopologyChangedException);
            DEFINE_PROTOCOL_EXCEPTION(TransactionException);
            DEFINE_PROTOCOL_EXCEPTION(TransactionNotActiveException);
            DEFINE_PROTOCOL_EXCEPTION(TransactionTimedOutException);
            DEFINE_PROTOCOL_EXCEPTION(URISyntaxException);
            DEFINE_PROTOCOL_EXCEPTION(UTFDataFormatException);
            DEFINE_PROTOCOL_EXCEPTION(UnsupportedOperationException);
            DEFINE_PROTOCOL_EXCEPTION(WrongTargetException);
            DEFINE_PROTOCOL_EXCEPTION(XAException);
            DEFINE_PROTOCOL_EXCEPTION(AccessControlException);
            DEFINE_PROTOCOL_EXCEPTION(LoginException);
            DEFINE_PROTOCOL_EXCEPTION(UnsupportedCallbackException);
            DEFINE_PROTOCOL_EXCEPTION(NoDataMemberInClusterException);
            DEFINE_PROTOCOL_EXCEPTION(ReplicatedMapCantBeCreatedOnLiteMemberException);
            DEFINE_PROTOCOL_EXCEPTION(MaxMessageSizeExceeded);
            DEFINE_PROTOCOL_EXCEPTION(WANReplicationQueueFullException);
            DEFINE_PROTOCOL_EXCEPTION(AssertionError);
            DEFINE_PROTOCOL_EXCEPTION(OutOfMemoryError);
            DEFINE_PROTOCOL_EXCEPTION(StackOverflowError);
            DEFINE_PROTOCOL_EXCEPTION(NativeOutOfMemoryError);
            DEFINE_PROTOCOL_EXCEPTION(ServiceNotFoundException);

            class HAZELCAST_API UndefinedErrorCodeException : public IException {
            public:
                UndefinedErrorCodeException(int32_t errorCode, int64_t correlationId, std::string details)
                : error(errorCode), messageCallId(correlationId), detailedErrorMessage(details) {
                }

                virtual ~UndefinedErrorCodeException() throw() {
                }

                int32_t getErrorCode() const {
                    return error;
                }

                int64_t getMessageCallId() const {
                    return messageCallId;
                }

                const std::string &getDetailedErrorMessage() const {
                    return detailedErrorMessage;
                }

            private:
                int32_t error;
                int64_t messageCallId;
                std::string detailedErrorMessage;
            };

            class HAZELCAST_API HazelcastClientNotActiveException : public IException {
            public:
                HazelcastClientNotActiveException(const std::string &source, const std::string &message) : IException(
                        source, message) {}
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_EXCEPTION_PROTOCOLEXCEPTIONS_H_
