/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

/*
 *
 *  Created on: May 17, 2016
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            ExceptionFactory::~ExceptionFactory() {
            }

            ClientExceptionFactory::ClientExceptionFactory() {
                registerException(ARRAY_INDEX_OUT_OF_BOUNDS, new ExceptionFactoryImpl<exception::ArrayIndexOutOfBoundsException>());
                registerException(ARRAY_STORE, new ExceptionFactoryImpl<exception::ArrayStoreException>());
                registerException(AUTHENTICATIONERROR, new ExceptionFactoryImpl<exception::AuthenticationException>());
                registerException(CACHE_NOT_EXISTS, new ExceptionFactoryImpl<exception::CacheNotExistsException>());
                registerException(CALLER_NOT_MEMBER, new ExceptionFactoryImpl<exception::CallerNotMemberException>());
                registerException(CANCELLATION, new ExceptionFactoryImpl<exception::CancellationException>());
                registerException(CLASS_CAST, new ExceptionFactoryImpl<exception::ClassCastException>());
                registerException(CLASS_NOT_FOUND, new ExceptionFactoryImpl<exception::ClassNotFoundException>());
                registerException(CONCURRENT_MODIFICATION, new ExceptionFactoryImpl<exception::ConcurrentModificationException>());
                registerException(CONFIG_MISMATCH, new ExceptionFactoryImpl<exception::ConfigMismatchException>());
                registerException(CONFIGURATION, new ExceptionFactoryImpl<exception::ConfigurationException>());
                registerException(DISTRIBUTED_OBJECT_DESTROYED, new ExceptionFactoryImpl<exception::DistributedObjectDestroyedException>());
                registerException(DUPLICATE_INSTANCE_NAME, new ExceptionFactoryImpl<exception::DuplicateInstanceNameException>());
                registerException(ENDOFFILE, new ExceptionFactoryImpl<exception::EOFException>());
                registerException(EXECUTION, new ExceptionFactoryImpl<exception::ExecutionException>());
                registerException(HAZELCAST, new ExceptionFactoryImpl<exception::HazelcastException>());
                registerException(HAZELCAST_INSTANCE_NOT_ACTIVE, new ExceptionFactoryImpl<exception::HazelcastInstanceNotActiveException>());
                registerException(HAZELCAST_OVERLOAD, new ExceptionFactoryImpl<exception::HazelcastOverloadException>());
                registerException(HAZELCAST_SERIALIZATION, new ExceptionFactoryImpl<exception::HazelcastSerializationException>());
                registerException(IO, new ExceptionFactoryImpl<exception::IOException>());
                registerException(ILLEGAL_ARGUMENT, new ExceptionFactoryImpl<exception::IllegalArgumentException>());
                registerException(ILLEGAL_ACCESS_EXCEPTION, new ExceptionFactoryImpl<exception::IllegalAccessException>());
                registerException(ILLEGAL_ACCESS_ERROR, new ExceptionFactoryImpl<exception::IllegalAccessError>());
                registerException(ILLEGAL_MONITOR_STATE, new ExceptionFactoryImpl<exception::IllegalMonitorStateException>());
                registerException(ILLEGAL_STATE, new ExceptionFactoryImpl<exception::IllegalStateException>());
                registerException(ILLEGAL_THREAD_STATE, new ExceptionFactoryImpl<exception::IllegalThreadStateException>());
                registerException(INDEX_OUT_OF_BOUNDS, new ExceptionFactoryImpl<exception::IndexOutOfBoundsException>());
                registerException(INTERRUPTED, new ExceptionFactoryImpl<exception::InterruptedException>());
                registerException(INVALID_ADDRESS, new ExceptionFactoryImpl<exception::InvalidAddressException>());
                registerException(INVALID_CONFIGURATION, new ExceptionFactoryImpl<exception::InvalidConfigurationException>());
                registerException(MEMBER_LEFT, new ExceptionFactoryImpl<exception::MemberLeftException>());
                registerException(NEGATIVE_ARRAY_SIZE, new ExceptionFactoryImpl<exception::NegativeArraySizeException>());
                registerException(NO_SUCH_ELEMENT, new ExceptionFactoryImpl<exception::NoSuchElementException>());
                registerException(NOT_SERIALIZABLE, new ExceptionFactoryImpl<exception::NotSerializableException>());
                registerException(NULL_POINTER, new ExceptionFactoryImpl<exception::NullPointerException>());
                registerException(OPERATION_TIMEOUT, new ExceptionFactoryImpl<exception::OperationTimeoutException>());
                registerException(PARTITION_MIGRATING, new ExceptionFactoryImpl<exception::PartitionMigratingException>());
                registerException(QUERY, new ExceptionFactoryImpl<exception::QueryException>());
                registerException(QUERY_RESULT_SIZE_EXCEEDED, new ExceptionFactoryImpl<exception::QueryResultSizeExceededException>());
                registerException(QUORUM, new ExceptionFactoryImpl<exception::QuorumException>());
                registerException(REACHED_MAX_SIZE, new ExceptionFactoryImpl<exception::ReachedMaxSizeException>());
                registerException(REJECTED_EXECUTION, new ExceptionFactoryImpl<exception::RejectedExecutionException>());
                registerException(REMOTE_MAP_REDUCE, new ExceptionFactoryImpl<exception::RemoteMapReduceException>());
                registerException(RESPONSE_ALREADY_SENT, new ExceptionFactoryImpl<exception::ResponseAlreadySentException>());
                registerException(RETRYABLE_HAZELCAST, new ExceptionFactoryImpl<exception::RetryableHazelcastException>());
                registerException(RETRYABLE_IO, new ExceptionFactoryImpl<exception::RetryableIOException>());
                registerException(RUNTIME, new ExceptionFactoryImpl<exception::RuntimeException>());
                registerException(SECURITY, new ExceptionFactoryImpl<exception::SecurityException>());
                registerException(SOCKET, new ExceptionFactoryImpl<exception::SocketException>());
                registerException(STALE_SEQUENCE, new ExceptionFactoryImpl<exception::StaleSequenceException>());
                registerException(TARGET_DISCONNECTED, new ExceptionFactoryImpl<exception::TargetDisconnectedException>());
                registerException(TARGET_NOT_MEMBER, new ExceptionFactoryImpl<exception::TargetNotMemberException>());
                registerException(TIMEOUT, new ExceptionFactoryImpl<exception::TimeoutException>());
                registerException(TOPIC_OVERLOAD, new ExceptionFactoryImpl<exception::TopicOverloadException>());
                registerException(TOPOLOGY_CHANGED, new ExceptionFactoryImpl<exception::TopologyChangedException>());
                registerException(TRANSACTION, new ExceptionFactoryImpl<exception::TransactionException>());
                registerException(TRANSACTION_NOT_ACTIVE, new ExceptionFactoryImpl<exception::TransactionNotActiveException>());
                registerException(TRANSACTION_TIMED_OUT, new ExceptionFactoryImpl<exception::TransactionTimedOutException>());
                registerException(URI_SYNTAX, new ExceptionFactoryImpl<exception::URISyntaxException>());
                registerException(UTF_DATA_FORMAT, new ExceptionFactoryImpl<exception::UTFDataFormatException>());
                registerException(UNSUPPORTED_OPERATION, new ExceptionFactoryImpl<exception::UnsupportedOperationException>());
                registerException(WRONG_TARGET, new ExceptionFactoryImpl<exception::WrongTargetException>());
                registerException(XA, new ExceptionFactoryImpl<exception::XAException>());
                registerException(ACCESS_CONTROL, new ExceptionFactoryImpl<exception::AccessControlException>());
                registerException(LOGIN, new ExceptionFactoryImpl<exception::LoginException>());
                registerException(UNSUPPORTED_CALLBACK, new ExceptionFactoryImpl<exception::UnsupportedCallbackException>());
                registerException(NO_DATA_MEMBER, new ExceptionFactoryImpl<exception::NoDataMemberInClusterException>());
                registerException(REPLICATED_MAP_CANT_BE_CREATED, new ExceptionFactoryImpl<exception::ReplicatedMapCantBeCreatedOnLiteMemberException>());
                registerException(MAX_MESSAGE_SIZE_EXCEEDED, new ExceptionFactoryImpl<exception::MaxMessageSizeExceeded>());
                registerException(WAN_REPLICATION_QUEUE_FULL, new ExceptionFactoryImpl<exception::WANReplicationQueueFullException>());
                registerException(ASSERTION_ERROR, new ExceptionFactoryImpl<exception::AssertionError>());
                registerException(OUT_OF_MEMORY_ERROR, new ExceptionFactoryImpl<exception::OutOfMemoryError>());
                registerException(STACK_OVERFLOW_ERROR, new ExceptionFactoryImpl<exception::StackOverflowError>());
                registerException(NATIVE_OUT_OF_MEMORY_ERROR, new ExceptionFactoryImpl<exception::NativeOutOfMemoryError>());
                registerException(SERVICE_NOT_FOUND, new ExceptionFactoryImpl<exception::ServiceNotFoundException>());
                registerException(CONSISTENCY_LOST, new ExceptionFactoryImpl<exception::ConsistencyLostException>());
            }

            ClientExceptionFactory::~ClientExceptionFactory() {
                // release memory for the factories
                for (std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it =
                        errorCodeToFactory.begin(); errorCodeToFactory.end() != it; ++it) {
                    delete(it->second);
                }
            }

            std::unique_ptr<exception::IException> ClientExceptionFactory::createException(const std::string &source,
                                                                                         protocol::ClientMessage &clientMessage) const {
                codec::ErrorCodec error = codec::ErrorCodec::decode(clientMessage);
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it = errorCodeToFactory.find(
                        error.errorCode);
                if (errorCodeToFactory.end() == it) {
                    return std::unique_ptr<exception::IException>(
                            new exception::UndefinedErrorCodeException(source, "",
                                                                       error.errorCode,
                                                                       clientMessage.getCorrelationId(),
                                                                       error.toString()));
                }

                return it->second->createException(source, error.message, error.toString(), error.causeErrorCode);
            }

            void ClientExceptionFactory::registerException(int32_t errorCode, ExceptionFactory *factory) {
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::iterator it = errorCodeToFactory.find(
                        errorCode);
                if (errorCodeToFactory.end() != it) {
                    char msg[100];
                    util::hz_snprintf(msg, 100, "Error code %d was already registered!!!", errorCode);
                    throw exception::IllegalStateException("ClientExceptionFactory::registerException", msg);
                }

                errorCodeToFactory[errorCode] = factory;
            }
        }
    }
}

