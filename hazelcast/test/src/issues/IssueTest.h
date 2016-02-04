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
// Created by sancar koyunlu on 21/04/14.
//


#ifndef HAZELCAST_IssueTest
#define HAZELCAST_IssueTest

#include "ClientTestSupport.h"
#include "hazelcast/client/EntryAdapter.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class IssueTest : public ClientTestSupport {

            public:
                IssueTest();

                ~IssueTest();
            protected:
                class Issue864MapListener : public hazelcast::client::EntryAdapter<int, int> {
                public:
                    Issue864MapListener(util::CountDownLatch &l);

                    virtual void entryAdded(const EntryEvent<int, int> &event);

                    virtual void entryUpdated(const EntryEvent<int, int> &event);

                private:
                    util::CountDownLatch &latch;
                };

                util::CountDownLatch latch;
                Issue864MapListener listener;
            };
        }
    }
}


#endif //HAZELCAST_IssueTest
