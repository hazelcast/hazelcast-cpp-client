/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_EXECEPTION_UNEXPECTEDMESSAGETYPEEXCEPTION_H_
#define HAZELCAST_CLIENT_EXECEPTION_UNEXPECTEDMESSAGETYPEEXCEPTION_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include "hazelcast/client/exception/IException.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * If the type expected by the message decoder is different from the received message type,
             * this exception is thrown.
             *
             */
            class HAZELCAST_API UnexpectedMessageTypeException : public hazelcast::client::exception::IException {
            public:
                /**
                 * Constructor
                 */
                UnexpectedMessageTypeException(const std::string &source, int actualType, int expectedType);

                /**
                 * Destructor
                 */
                virtual ~UnexpectedMessageTypeException() throw();

                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();
            private:
                int actual;
                int expected;
                std::string message;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_EXECEPTION_UNEXPECTEDMESSAGETYPEEXCEPTION_H_

