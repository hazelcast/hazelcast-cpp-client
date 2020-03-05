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
 * ErrorCodec.h
 *
 *  Created on: Apr 13, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_PROTOCOL_STACKTRACEELEMENT_H_
#define HAZELCAST_CLIENT_PROTOCOL_STACKTRACEELEMENT_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <string>
#include <memory>
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                class HAZELCAST_API StackTraceElement {
                public:
                    StackTraceElement();

                    StackTraceElement(const std::string &className, const std::string &method,
                                      std::unique_ptr<std::string> &file, int line);

                    StackTraceElement(const StackTraceElement &rhs);

                    StackTraceElement &operator=(const StackTraceElement &rhs);

                    const std::string &getDeclaringClass() const;

                    const std::string &getMethodName() const;

                    const std::string &getFileName() const;

                    int getLineNumber() const;

                private:
                    static const std::string EMPTY_STRING;
                    std::string declaringClass;
                    std::string methodName;
                    std::unique_ptr<std::string> fileName;
                    int lineNumber;
                };

                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_PROTOCOL_STACKTRACEELEMENT_H_ */
