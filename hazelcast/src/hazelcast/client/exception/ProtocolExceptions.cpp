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

            TargetNotMemberException::TargetNotMemberException(const std::string &source, const std::string &message,
                                                               const std::string &details, int32_t causeCode)
                    : RetryableHazelcastException(source, message, details, causeCode) {
                errorCode = protocol::TARGET_NOT_MEMBER;
            }

            TargetNotMemberException::TargetNotMemberException(const std::string &source, const std::string &message)
                    : RetryableHazelcastException(source, message) {
                errorCode = protocol::TARGET_NOT_MEMBER;
            }

            TargetNotMemberException::TargetNotMemberException(const std::string &source, const std::string &message,
                                                               int32_t causeCode) : RetryableHazelcastException(source,
                                                                                                                message,
                                                                                                                causeCode) {}
        }
    }
}

