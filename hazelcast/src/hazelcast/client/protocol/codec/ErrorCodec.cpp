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
 * ErrorCodec.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: ihsan
 */

#include <sstream>
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                ErrorCodec ErrorCodec::decode(ClientMessage &clientMessage) {
                    return ErrorCodec(clientMessage);
                }

                ErrorCodec::ErrorCodec(ClientMessage &clientMessage) {
                    assert(ErrorCodec::TYPE == clientMessage.getMessageType());

                    errorCode = clientMessage.getInt32();
                    className = clientMessage.getStringUtf8();
                    message = clientMessage.getNullable<std::string>();
                    stackTrace = clientMessage.getArray<StackTraceElement>();
                    causeErrorCode = clientMessage.getInt32();
                    causeClassName = clientMessage.getNullable<std::string>();
                }

                std::string ErrorCodec::toString() const {
                    std::ostringstream out;
                    out << "Error code:" << errorCode << ", Class name that generated the error:" << className <<
                    ", ";
                    if (NULL != message.get()) {
                        out << *message;
                    }
                    out << std::endl;
                    for (std::vector<StackTraceElement>::const_iterator it = stackTrace.begin();
                         it != stackTrace.end(); ++it) {
                        out << "\t" << (*it) << std::endl;
                    }

                    out << std::endl << "Cause error code:" << causeErrorCode << std::endl;
                    if (NULL != causeClassName.get()) {
                        out << "Caused by:" << *causeClassName;
                    }

                    return out.str();
                }

                ErrorCodec::ErrorCodec(const ErrorCodec &rhs) {
                    errorCode = rhs.errorCode;
                    className = rhs.className;
                    if (NULL != rhs.message.get()) {
                        message = std::auto_ptr<std::string>(new std::string(*rhs.message));
                    }
                    stackTrace = rhs.stackTrace;
                    causeErrorCode = rhs.causeErrorCode;
                    if (NULL != rhs.causeClassName.get()) {
                        causeClassName = std::auto_ptr<std::string>(new std::string(*rhs.causeClassName));
                    }
                }
            }
        }
    }
}
