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
// Created by sancar koyunlu on 23/01/14.
//


#ifndef HAZELCAST_Credentials
#define HAZELCAST_Credentials

#include "hazelcast/client/serialization/Portable.h"
#include <string>

namespace hazelcast {
    namespace client {
        /**
         * Credentials is a container object for endpoint (Members and Clients)
         * security attributes.
         *
         * It is used on authentication process by LoginModule.
         */
        class HAZELCAST_API Credentials : public serialization::Portable {
        public:
            /**
             * Returns IP address of endpoint.
             *
             * @return endpoint address
             */
            virtual const std::string &getEndpoint() const = 0;

            /**
             * Sets IP address of endpoint.
             *
             * @param endpoint address
             */
            virtual void setEndpoint(const std::string &endpoint) = 0;

            /**
             * Returns principal of endpoint.
             *
             * @return endpoint principal
             */
            virtual const std::string &getPrincipal() const = 0;

            /**
             *  Destructor
             */
            virtual ~Credentials() {

            }

        };
    }
}


#endif //HAZELCAST_Credentials

