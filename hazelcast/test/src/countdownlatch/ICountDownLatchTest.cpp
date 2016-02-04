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
// Created by sancar koyunlu on 9/5/13.



#include "ICountDownLatchTest.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ICountDownLatchTest::ICountDownLatchTest()
            : instance(*g_srvFactory)
            , client(getNewClient())
            , l(new ICountDownLatch(client->getICountDownLatch("ICountDownLatchTest"))) {
            }

            ICountDownLatchTest::~ICountDownLatchTest() {
            }

            void testLatchThread(util::ThreadArgs &args) {
                ICountDownLatch *l = (ICountDownLatch *) args.arg0;
                for (int i = 0; i < 20; i++) {
                    l->countDown();
                }
            }

            TEST_F(ICountDownLatchTest, testLatch) {
                ASSERT_TRUE(l->trySetCount(20));
                ASSERT_FALSE(l->trySetCount(10));
                ASSERT_EQ(20, l->getCount());

                util::Thread t(testLatchThread, l.get());

                ASSERT_TRUE(l->await(10 * 1000));
            }

        }
    }
}


