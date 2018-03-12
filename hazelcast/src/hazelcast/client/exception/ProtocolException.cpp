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
            ProtocolException::ProtocolException(const std::string &source, const std::string &message,
                                                 const std::string &details,
                                                 int32_t errorNo,
                                                 int32_t causeCode)
                    : IException(source, message + ". Details:" + details), errorCode(errorNo),
                      causeErrorCode(causeCode) {
            }

            ProtocolException::ProtocolException(const std::string &source, const std::string &message,
                                                 int32_t errorNo, int32_t causeCode)
                    : IException(source, message), errorCode(errorNo), causeErrorCode(causeCode) {
            }

            ProtocolException::ProtocolException(const std::string &source, const std::string &message)
                    : IException(source, message), errorCode(-1), causeErrorCode(-1) {
            }

            ProtocolException::ProtocolException(const std::string &source, const std::string &message,
                                                 const boost::shared_ptr<IException> &cause) : IException(source,
                                                                                                          message,
                                                                                                          cause),
                                                                                               errorCode(-1),
                                                                                               causeErrorCode(-1) {}

            int32_t ProtocolException::getErrorCode() const {
                return errorCode;
            }

            int32_t ProtocolException::getCauseErrorCode() const {
                return causeErrorCode;
            }

            std::auto_ptr<IException> ProtocolException::clone() const {
                return std::auto_ptr<IException>(new ProtocolException(*this));
            }

            void ProtocolException::raise() const {
                throw *this;
            }

            UndefinedErrorCodeException::UndefinedErrorCodeException(int32_t errorCode, int64_t correlationId,
                                                                     std::string details)
                    : error(errorCode), messageCallId(correlationId), detailedErrorMessage(details) {
            }

            int32_t UndefinedErrorCodeException::getErrorCode() const {
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

            HazelcastClientNotActiveException::HazelcastClientNotActiveException(const std::string &source,
                                                                                 const std::string &message)
                    : IException(
                    source, message) {}

            HazelcastClientNotActiveException::~HazelcastClientNotActiveException() throw() {

            }

            HazelcastClientOfflineException::HazelcastClientOfflineException(const std::string &source,
                                                                             const std::string &message)
                    : IllegalStateException(source, message) {}

            HazelcastClientOfflineException::~HazelcastClientOfflineException() throw() {
            }

            UnknownHostException::UnknownHostException(const std::string &source, const std::string &message)
                    : IException(source, message) {
            }
        }
    }
}

