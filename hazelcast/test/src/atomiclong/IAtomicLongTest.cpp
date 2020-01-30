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
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IAtomicLong.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class IAtomicLongTest : public ClientTestSupport {
            public:
                IAtomicLongTest();

            protected:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance;
                }

                ClientConfig clientConfig;
                HazelcastClient client;
                std::unique_ptr<IAtomicLong> l;

                static HazelcastServer *instance;
            };

            HazelcastServer *IAtomicLongTest::instance = NULL;

            IAtomicLongTest::IAtomicLongTest() : client(getNewClient()),
                                                 l(new IAtomicLong(client.getIAtomicLong(getTestName()))) {
                l->set(0);
            }

            TEST_F(IAtomicLongTest, testSync) {
                ASSERT_EQ(0, l->getAndAdd(2));
                ASSERT_EQ(2, l->get());
                l->set(5);
                ASSERT_EQ(5, l->get());
                ASSERT_EQ(8, l->addAndGet(3));
                ASSERT_FALSE(l->compareAndSet(7, 4));
                ASSERT_EQ(8, l->get());
                ASSERT_TRUE(l->compareAndSet(8, 4));
                ASSERT_EQ(4, l->get());
                ASSERT_EQ(3, l->decrementAndGet());
                ASSERT_EQ(3, l->getAndIncrement());
                ASSERT_EQ(4, l->getAndSet(9));
                ASSERT_EQ(10, l->incrementAndGet());
            }

            TEST_F(IAtomicLongTest, testAsync) {
                std::shared_ptr<ICompletableFuture<int64_t> > future = l->getAndAddAsync(10);
                ASSERT_EQ(0, *future->get());

                std::shared_ptr<ICompletableFuture<bool> > booleanFuture = l->compareAndSetAsync(10, 42);
                ASSERT_TRUE(booleanFuture->get());

                future = l->getAsync();
                ASSERT_EQ(42, *future->get());

                future = l->incrementAndGetAsync();
                ASSERT_EQ(43, *future->get());

                future = l->addAndGetAsync(-13);
                ASSERT_EQ(30, *future->get());

                future = l->decrementAndGetAsync();
                ASSERT_EQ(29, *future->get());

                future = l->getAndSetAsync(15);
                ASSERT_EQ(29, *future->get());

                future = l->getAsync();
                ASSERT_EQ(15, *future->get());

                future = l->getAndIncrementAsync();
                ASSERT_EQ(15, *future->get());

                future = l->getAsync();
                ASSERT_EQ(16, *future->get());

                std::shared_ptr<ICompletableFuture<void> > voidFuture = l->setAsync(55);
                voidFuture->get();

                future = l->getAsync();
                ASSERT_EQ(55, *future->get());
            }
        }
    }
}
