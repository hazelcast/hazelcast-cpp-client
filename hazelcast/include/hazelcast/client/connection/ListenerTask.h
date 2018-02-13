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
// Created by sancar koyunlu on 24/12/13.
//

#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class HAZELCAST_API ListenerTask {
            public:
                virtual void run() = 0;

                virtual ~ListenerTask();
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask

