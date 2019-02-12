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
// Created by sancar koyunlu on 9/13/13.

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"
#include "../serialization/Employee.h"

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class RingbufferTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    rb = client->getRingbuffer<Employee>("rb-1");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete clientConfig;
                    delete instance;

                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static boost::shared_ptr<Ringbuffer<Employee> > rb;

                static const int64_t CAPACITY;
            };

            const int64_t RingbufferTest::CAPACITY = 10;

            HazelcastServer *RingbufferTest::instance = NULL;
            ClientConfig *RingbufferTest::clientConfig = NULL;
            HazelcastClient *RingbufferTest::client = NULL;
            boost::shared_ptr<Ringbuffer<Employee> > RingbufferTest::rb = boost::shared_ptr<Ringbuffer<Employee> >();

            TEST_F(RingbufferTest, testAPI) {
                ASSERT_EQ(CAPACITY, rb->capacity());
                ASSERT_EQ(0, rb->headSequence());
                ASSERT_EQ(-1, rb->tailSequence());
                ASSERT_EQ(0, rb->size());
                ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                ASSERT_THROW(rb->readOne(-1), exception::IllegalArgumentException);
                ASSERT_THROW(rb->readOne(1), exception::IllegalArgumentException);

                Employee employee1("First", 10);
                Employee employee2("Second", 20);

                ASSERT_EQ(0, rb->add(employee1));
                ASSERT_EQ(CAPACITY, rb->capacity());
                ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                ASSERT_EQ(0, rb->headSequence());
                ASSERT_EQ(0, rb->tailSequence());
                ASSERT_EQ(1, rb->size());
                ASSERT_EQ(employee1, *rb->readOne(0));
                ASSERT_THROW(rb->readOne(2), exception::IllegalArgumentException);

                ASSERT_EQ(1, rb->add(employee2));
                ASSERT_EQ(CAPACITY, rb->capacity());
                ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                ASSERT_EQ(0, rb->headSequence());
                ASSERT_EQ(1, rb->tailSequence());
                ASSERT_EQ(2, rb->size());
                ASSERT_EQ(employee1, *rb->readOne(0));
                ASSERT_EQ(employee2, *rb->readOne(1));
                ASSERT_THROW(*rb->readOne(3), exception::IllegalArgumentException);

                // insert many employees to fill the ringbuffer capacity
                for (int i = 0; i < CAPACITY - 2; ++i) {
                    Employee eleman("name", 10 * (i + 2));
                    ASSERT_EQ(i + 2, rb->add(eleman));
                    ASSERT_EQ(CAPACITY, rb->capacity());
                    ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                    ASSERT_EQ(0, rb->headSequence());
                    ASSERT_EQ(i + 2, rb->tailSequence());
                    ASSERT_EQ(i + 3, rb->size());
                    ASSERT_EQ(eleman, *rb->readOne(i + 2));
                }

                // verify that the head element is overriden on the first add
                Employee latestEmployee("latest employee", 100);
                ASSERT_EQ(CAPACITY, rb->add(latestEmployee));
                ASSERT_EQ(CAPACITY, rb->capacity());
                ASSERT_EQ(CAPACITY, rb->remainingCapacity());
                ASSERT_EQ(1, rb->headSequence());
                ASSERT_EQ(CAPACITY, rb->tailSequence());
                ASSERT_EQ(CAPACITY, rb->size());
                ASSERT_EQ(latestEmployee, *rb->readOne(CAPACITY));
            }
        }
    }
}

