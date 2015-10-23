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
// Created by sancar koyunlu on 04/04/14.
//


#ifndef HAZELCAST_ExceptionHandler
#define HAZELCAST_ExceptionHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            namespace pimpl{
                class HAZELCAST_API ExceptionHandler {
                public:
                    /**
                    * InternalAPI java side class name of interrupted exception
                    */
                    static std::string INTERRUPTED;
                    /**
                    * InternalAPI java side class name of instance not active exception
                    */

                    static std::string INSTANCE_NOT_ACTIVE;
                    /**
                    * InternalAPI rethrows the exception with appropriate type
                    *
                    * @param exceptionName java(node) side exceptionName
                    * @param message exception cause message
                    */
                    static void rethrow(const std::string &exceptionName, const std::string &message);

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ExceptionHandler

