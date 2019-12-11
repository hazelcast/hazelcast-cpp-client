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
// Created by sancar koyunlu on 25/12/13.
//
#ifndef HAZELCAST_OListener
#define HAZELCAST_OListener

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/Mutex.h"
#include <map>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class HAZELCAST_API OutSelector : public IOSelector {
            public:
                OutSelector(ClientConnectionManagerImpl &connectionManager, const config::SocketOptions &socketOptions);

                void listenInternal();

                bool start();

                virtual const std::string getName() const;

            private:
                util::SocketSet wakeUpSocketSet;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_OListener

