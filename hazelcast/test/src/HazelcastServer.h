/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once
#include <atomic>

#include "HazelcastServerFactory.h"

using namespace hazelcast::client::test::remote;

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastServer {
            public:

                HazelcastServer(HazelcastServerFactory &factory);

                /**
                 * @returns true if the server were not started before and it is now started successfully
                 */
                bool start();

                bool shutdown();

                bool terminate();

                ~HazelcastServer();

                bool setAttributes(int memberStartOrder);

                const remote::Member &getMember() const;

            private:
                HazelcastServerFactory & factory;
                std::atomic_bool isStarted;
                std::atomic_bool isShutdown;
                remote::Member member;
                std::shared_ptr<util::ILogger> logger;
            };
        }
    }
}


