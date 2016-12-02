/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 01 Dec 2016.
//

#ifndef HAZELCAST_CLIENT_EXCEPTION_CLIENTSERVICENOTFOUNDEXCEPTION_H_
#define HAZELCAST_CLIENT_EXCEPTION_CLIENTSERVICENOTFOUNDEXCEPTION_H_

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            class HAZELCAST_API ClientServiceNotFoundException : public exception::IException {
            public:
                ClientServiceNotFoundException(const std::string &source, const std::string &message) : IException(source, message) {}
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_EXCEPTION_CLIENTSERVICENOTFOUNDEXCEPTION_H_
