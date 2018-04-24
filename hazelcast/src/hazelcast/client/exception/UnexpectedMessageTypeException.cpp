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
// Created by ihsan demir on 10/11/15.

#include "hazelcast/client/exception/UnexpectedMessageTypeException.h"

#include <sstream>
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            UnexpectedMessageTypeException::UnexpectedMessageTypeException(const std::string &source,
                                                                           int actualType, int expectedType)
                    : hazelcast::client::exception::IException("UnexpectedMessageTypeException", source, "",
                                                               protocol::UNEXPECTED_MESSAGE_TYPE) {
                std::ostringstream os;
                os << "[" << hazelcast::client::exception::IException::what() << "] Expected type:" << expected <<
                   ", but message type is:" << actual;
                message = os.str();
            }

            UnexpectedMessageTypeException::~UnexpectedMessageTypeException()  throw() {

            }

            char const *UnexpectedMessageTypeException::what() const throw() {
                return message.c_str();
            }
        }
    }
}

