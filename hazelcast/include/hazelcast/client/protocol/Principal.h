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
// Created by sancar koyunlu on 5/20/13.

#ifndef HAZELCAST_PRINCIPAL
#define HAZELCAST_PRINCIPAL

#include <string>
#include <boost/shared_ptr.hpp>
#include <ostream>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            class HAZELCAST_API Principal {
            public:
                Principal(std::auto_ptr<std::string> id, std::auto_ptr<std::string> owner);

                const std::string *getUuid() const;

                const std::string *getOwnerUuid() const;

                bool operator==(const Principal &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const Principal &principal);

            private:
                boost::shared_ptr<std::string> uuid;
                boost::shared_ptr<std::string> ownerUuid;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PRINCIPAL

