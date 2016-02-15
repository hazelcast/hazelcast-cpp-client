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
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "IAtomicLongTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            IAtomicLongTest::IAtomicLongTest() : instance(*g_srvFactory), client(getNewClient()),
                                                 atom(new IAtomicLong(client->getIAtomicLong("clientAtomicLong"))) {
                atom->set(0);
            }

            IAtomicLongTest::~IAtomicLongTest() {
                atom.reset();
                client.reset();
                instance.shutdown();
            }

            TEST_F(IAtomicLongTest, testAtomicLong) {
                ASSERT_EQ(0, atom->getAndAdd(2));
                ASSERT_EQ(2, atom->get());
                atom->set(5);
                ASSERT_EQ(5, atom->get());
                ASSERT_EQ(8, atom->addAndGet(3));
                ASSERT_FALSE(atom->compareAndSet(7, 4));
                ASSERT_EQ(8, atom->get());
                ASSERT_TRUE(atom->compareAndSet(8, 4));
                ASSERT_EQ(4, atom->get());
                ASSERT_EQ(3, atom->decrementAndGet());
                ASSERT_EQ(3, atom->getAndIncrement());
                ASSERT_EQ(4, atom->getAndSet(9));
                ASSERT_EQ(10, atom->incrementAndGet());
            }

        }
    }
}
