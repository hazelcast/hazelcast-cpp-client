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
//
// Created by ihsan demir on 5/11/15.
//

#include "hazelcast/client/protocol/codec/StackTraceElement.h"
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const std::string StackTraceElement::EMPTY_STRING("");

                StackTraceElement::StackTraceElement() : fileName((std::string *)NULL) {
                }

                StackTraceElement::StackTraceElement(const std::string &className, const std::string &method,
                                  std::unique_ptr<std::string> &file, int line) : declaringClass(className),
                                                                                 methodName(method),
                                                                                 fileName(std::move(file)),
                                                                                 lineNumber(line) { }


                StackTraceElement::StackTraceElement(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                }

                StackTraceElement &StackTraceElement::operator=(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                    return *this;
                }

                const std::string &StackTraceElement::getDeclaringClass() const {
                    return declaringClass;
                }

                const std::string &StackTraceElement::getMethodName() const {
                    return methodName;
                }

                const std::string &StackTraceElement::getFileName() const {
                    if (NULL == fileName.get()) {
                        return EMPTY_STRING;
                    }

                    return *fileName;
                }

                int StackTraceElement::getLineNumber() const {
                    return lineNumber;
                }

                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace) {
                    return out << trace.getFileName() << " line " << trace.getLineNumber() << " :" <<
                           trace.getDeclaringClass() << "." << trace.getMethodName();
                }
            }
        }
    }
}
