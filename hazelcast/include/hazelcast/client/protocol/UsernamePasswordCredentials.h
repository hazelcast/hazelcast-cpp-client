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
// Created by sancar koyunlu on 5/7/13.

#ifndef HAZELCAST_CREDENTIALS
#define HAZELCAST_CREDENTIALS

#include <string>
#include <vector>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace protocol {
            /**
             * Credentials is a container object for endpoint (Members and Clients)
             * security attributes.
             *
             * It is used on authentication process.
             */
            class HAZELCAST_API UsernamePasswordCredentials {
            public:

                UsernamePasswordCredentials(const std::string &principal, const std::string &password);

                const std::string &getPrincipal() const;

                const std::string &getPassword() const;

            private:
                std::string principal;
                std::string password;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


#endif //HAZELCAST_CREDENTIALS

