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

#include "hazelcast/client/exception/ProtocolExceptions.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            UndefinedErrorCodeException::UndefinedErrorCodeException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t errorCode, int64_t correlationId,
                                                                     std::string details)
                    : IException("UndefinedErrorCodeException", source, message, protocol::UNDEFINED), error(errorCode),
                      messageCallId(correlationId),
                      detailedErrorMessage(details) {
            }

            int32_t UndefinedErrorCodeException::getUndefinedErrorCode() const {
                return error;
            }

            int64_t UndefinedErrorCodeException::getMessageCallId() const {
                return messageCallId;
            }

            const std::string &UndefinedErrorCodeException::getDetailedErrorMessage() const {
                return detailedErrorMessage;
            }

            UndefinedErrorCodeException::~UndefinedErrorCodeException() throw() {
            }

            std::auto_ptr<IException> UndefinedErrorCodeException::clone() const {
                return std::auto_ptr<IException>(new UndefinedErrorCodeException(*this));
            }

            void UndefinedErrorCodeException::raise() const {
                throw *this;
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const std::string &details, int32_t causeCode)
                    : IException("RetryableHazelcastException", source, message, details, protocol::RETRYABLE_HAZELCAST,
                                 causeCode), HazelcastException(source, message, details, causeCode) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST),
                      HazelcastException(source, message) {
            }

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     int32_t causeCode) : IException(
                    "RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, causeCode),
                                                                                          HazelcastException(source,
                                                                                                             message,
                                                                                                             causeCode) {}

            RetryableHazelcastException::RetryableHazelcastException(const std::string &source,
                                                                     const std::string &message,
                                                                     const boost::shared_ptr<IException> &cause)
                    : IException("RetryableHazelcastException", source, message, protocol::RETRYABLE_HAZELCAST, cause),
                      HazelcastException(source, message, cause) {}

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     const std::string &details, int32_t causeCode)
                    : IException("MemberLeftException", source, message, details, protocol::MEMBER_LEFT, causeCode),
                      ExecutionException(source, message, details, causeCode),
                      RetryableHazelcastException(source, message, details, causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message,
                                                     int32_t causeCode) : IException("MemberLeftException", source,
                                                                                     message, protocol::MEMBER_LEFT,
                                                                                     causeCode),
                                                                          ExecutionException(source, message,
                                                                                             causeCode),
                                                                          RetryableHazelcastException(source, message,
                                                                                                      causeCode) {
            }

            MemberLeftException::MemberLeftException(const std::string &source, const std::string &message)
                    : IException("MemberLeftException", source, message, protocol::MEMBER_LEFT),
                      ExecutionException(source, message), RetryableHazelcastException(source, message) {
            }

            void MemberLeftException::raise() const {
                throw *this;
            }

            std::auto_ptr<IException> MemberLeftException::clone() const {
                return std::auto_ptr<IException>(new MemberLeftException(*this));
            }

        }
    }
}

