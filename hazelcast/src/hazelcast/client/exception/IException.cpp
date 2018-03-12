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
// Created by msk on 3/13/13.

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            IException::IException() {
            }

            IException::IException(const std::string& source, const std::string& message) : src(source), msg(message) {
                report = "ExceptionMessage {" + message + "} at " + source;
            }

            IException::IException(const std::string &source, const std::string &message,
                                   const boost::shared_ptr<IException> &cause) : src(source), msg(message),
                                                                                 cause(cause) {
                if (cause.get()) {
                    std::ostringstream out;
                    out << "ExceptionMessage {" << message + " Caused by:" << *cause << "} at " << source;
                } else {
                    report = "ExceptionMessage {" + message + "} at " + source;
                }
            }

            IException::~IException() throw() {
            }

            char const *IException::what() const throw() {
                return report.c_str();
            }

            const std::string &IException::getSource() const {
                return src;
            }

            const std::string &IException::getMessage() const {
                return msg;
            }

            void IException::raise() const {
                throw *this;
            }

            std::ostream &operator<<(std::ostream &os, const IException &exception) {
                os << exception.what();
                return os;
            }

            const boost::shared_ptr<IException> &IException::getCause() const {
                return cause;
            }

            std::auto_ptr<IException> IException::clone() const {
                return std::auto_ptr<IException>(new IException(*this));
            }

        }
    }
}
