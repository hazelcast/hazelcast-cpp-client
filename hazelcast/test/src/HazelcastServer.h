/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//  hazelcastInstance.h
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef __hazelcast__hazelcastInstance__
#define __hazelcast__hazelcastInstance__

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastServerFactory;

            class HazelcastServer {
            public:
                static const int DEFAULT_RETRY_COUNT = 3;

                HazelcastServer(HazelcastServerFactory &);

                /**
                 * @returns true if the server were not started before and it is now started successfully
                 */
                bool start();

                bool shutdown();

                ~HazelcastServer();

            private:

                HazelcastServerFactory & factory;
                int id;
                bool isShutDown;

            };
        }
    }
}

#endif /* defined(__hazelcast__hazelcastInstance__) */

