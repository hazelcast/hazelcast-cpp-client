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
// Created by sancar koyunlu on 07/04/14.
//

#include "hazelcast/client/exception/InterruptedException.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/exception/pimpl/ExceptionHandler.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            namespace pimpl{
                std::string ExceptionHandler::INTERRUPTED = "InterruptedException";
                std::string ExceptionHandler::INSTANCE_NOT_ACTIVE = "HazelcastInstanceNotActiveException";

                void ExceptionHandler::rethrow(const std::string &exceptionName, const std::string &message) {
                    if (INTERRUPTED == exceptionName) {
                        exception::InterruptedException exception(exceptionName, message);
                        throw  exception;
                    } else if (INSTANCE_NOT_ACTIVE == exceptionName) {
                        exception::InstanceNotActiveException exception(message);
                        throw  exception;
                    } else {
                        exception::IException exception("Server:", message);
                        throw  exception;
                    }
                }
            }
        }
    }
}
