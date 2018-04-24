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

#ifndef HAZELCAST_CLIENT_IMPL_BUILDINFO_H_
#define HAZELCAST_CLIENT_IMPL_BUILDINFO_H_

#include <string>
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace impl {
            class HAZELCAST_API BuildInfo {
            public:
                static const int UNKNOWN_HAZELCAST_VERSION = -1;

                static int calculateVersion(const std::string &version);

            private:
                // major.minor.patch-RC-SNAPSHOT
                static const int MAJOR_VERSION_MULTIPLIER = 10000;
                static const int MINOR_VERSION_MULTIPLIER = 100;
                static const size_t PATCH_TOKEN_INDEX = 2;

                std::string version;
                std::string build;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_IMPL_BUILDINFO_H_

