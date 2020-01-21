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
#include "HazelcastServerFactory.h"

#include <boost/make_shared.hpp>

#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"
#include "../serialization/Employee.h"
#include "StartsWithStringFilter.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace ringbuffer {
                class RingbufferTest : public ClientTestSupport {
                public:
                    RingbufferTest() {
                        for (int i = 0; i < 11; ++i) {
                            std::ostringstream out;
                            out << i;
                            items.push_back(out.str());
                        }
                    }

                protected:
                    class ReadOneWithLatchTask : public util::Runnable {
                    public:
                        ReadOneWithLatchTask(const boost::shared_ptr<Ringbuffer<std::string> > &clientRingbuffer,
                                             const boost::shared_ptr<CountDownLatch> &latch) : clientRingbuffer(
                                clientRingbuffer), latch(latch) {}

                        virtual const string getName() const {
                            return "ReadOneWithLatchTask";
                        }

                        virtual void run() {
                            try {
                                clientRingbuffer->readOne(0);
                            } catch (exception::InterruptedException &e) {
                                std::cerr << e;
                            } catch (exception::StaleSequenceException &) {
                                latch->countDown();
                            }
                        }

                    private:
                        const boost::shared_ptr<Ringbuffer<std::string> > clientRingbuffer;
                        const boost::shared_ptr<util::CountDownLatch> latch;
                        static const int CAPACITY;
                    };

                    virtual void SetUp() {
                        std::string testName = getTestName();
                        clientRingbuffer = client->getRingbuffer<std::string>(testName);
                        client2Ringbuffer = client2->getRingbuffer<std::string>(testName);
                    }

                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                        client2 = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete client2;
                        delete instance;

                        client = NULL;
                        client2 = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static HazelcastClient *client;
                    static HazelcastClient *client2;
                    boost::shared_ptr<Ringbuffer<std::string> > clientRingbuffer;
                    boost::shared_ptr<Ringbuffer<std::string> > client2Ringbuffer;
                    std::vector<std::string> items;

                    static const int64_t CAPACITY;
                };

                const int64_t RingbufferTest::CAPACITY = 10;

                HazelcastServer *RingbufferTest::instance = NULL;
                HazelcastClient *RingbufferTest::client = NULL;
                HazelcastClient *RingbufferTest::client2 = NULL;

                TEST_F(RingbufferTest, testAPI) {
                    boost::shared_ptr<Ringbuffer<Employee> > rb = client->getRingbuffer<Employee>(getTestName() + "2");
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

                TEST_F(RingbufferTest, readManyAsync_whenHitsStale_shouldNotBeBlocked) {
                    boost::shared_ptr<ICompletableFuture<client::ringbuffer::ReadResultSet<std::string> > > f = clientRingbuffer->readManyAsync<void>(
                            0, 1, 10, NULL);
                    client2Ringbuffer->addAllAsync(items, Ringbuffer<std::string>::OVERWRITE);
                    try {
                        f->get();
                    } catch (exception::ExecutionException &e) {
                        boost::shared_ptr<exception::IException> cause = e.getCause();
                        ASSERT_NOTNULL(cause.get(), exception::IException);
                        ASSERT_THROW(cause->raise(), exception::StaleSequenceException);
                    }
                }

                TEST_F(RingbufferTest, readOne_whenHitsStale_shouldNotBeBlocked) {
                    boost::shared_ptr<hazelcast::util::CountDownLatch> latch = boost::make_shared<util::CountDownLatch>(
                            1);
                    util::Thread consumer(boost::make_shared<ReadOneWithLatchTask>(clientRingbuffer, latch),
                                          getLogger());
                    consumer.start();
                    client2Ringbuffer->addAllAsync(items, Ringbuffer<std::string>::OVERWRITE);
                    ASSERT_OPEN_EVENTUALLY(*latch);
                }

                TEST_F(RingbufferTest, headSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add("foo");
                    }

                    ASSERT_EQ(client2Ringbuffer->headSequence(), clientRingbuffer->headSequence());
                }

                TEST_F(RingbufferTest, tailSequence) {
                    for (int k = 0; k < 2 * CAPACITY; k++) {
                        client2Ringbuffer->add("foo");
                    }

                    ASSERT_EQ(client2Ringbuffer->tailSequence(), clientRingbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, size) {
                    client2Ringbuffer->add("foo");

                    ASSERT_EQ(client2Ringbuffer->tailSequence(), clientRingbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, capacity) {
                    ASSERT_EQ(client2Ringbuffer->capacity(), clientRingbuffer->capacity());
                }

                TEST_F(RingbufferTest, remainingCapacity) {
                    client2Ringbuffer->add("foo");

                    ASSERT_EQ(client2Ringbuffer->remainingCapacity(), clientRingbuffer->remainingCapacity());
                }

                TEST_F(RingbufferTest, add) {
                    clientRingbuffer->add("foo");

                    std::auto_ptr<std::string> value = client2Ringbuffer->readOne(0);
                    ASSERT_EQ_PTR("foo", value.get(), std::string);
                }

                TEST_F(RingbufferTest, addAsync) {
                    boost::shared_ptr<ICompletableFuture<int64_t> > f = clientRingbuffer->addAsync("foo",
                                                                                                   Ringbuffer<std::string>::OVERWRITE);
                    boost::shared_ptr<int64_t> result = f->get();

                    ASSERT_EQ_PTR(client2Ringbuffer->headSequence(), result.get(), int64_t);
                    ASSERT_EQ_PTR("foo", client2Ringbuffer->readOne(0).get(), std::string);
                    ASSERT_EQ(0, client2Ringbuffer->headSequence());
                    ASSERT_EQ(0, client2Ringbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, addAllAsync) {
                    std::vector<std::string> items;
                    items.push_back("foo");
                    items.push_back("bar");
                    boost::shared_ptr<ICompletableFuture<int64_t> > f = clientRingbuffer->addAllAsync(items,
                                                                                                      Ringbuffer<std::string>::OVERWRITE);
                    boost::shared_ptr<int64_t> result = f->get();

                    ASSERT_EQ_PTR(client2Ringbuffer->tailSequence(), result.get(), int64_t);
                    ASSERT_EQ_PTR("foo", client2Ringbuffer->readOne(0).get(), std::string);
                    ASSERT_EQ_PTR("bar", client2Ringbuffer->readOne(1).get(), std::string);
                    ASSERT_EQ(0, client2Ringbuffer->headSequence());
                    ASSERT_EQ(1, client2Ringbuffer->tailSequence());
                }

                TEST_F(RingbufferTest, readOne) {
                    client2Ringbuffer->add("foo");
                    ASSERT_EQ_PTR("foo", clientRingbuffer->readOne(0).get(), std::string);
                }

                TEST_F(RingbufferTest, readManyAsync_noFilter) {
                    client2Ringbuffer->add("1");
                    client2Ringbuffer->add("2");
                    client2Ringbuffer->add("3");

                    boost::shared_ptr<ICompletableFuture<client::ringbuffer::ReadResultSet<std::string> > > f = clientRingbuffer->readManyAsync<void>(
                            0, 3, 3, NULL);
                    boost::shared_ptr<client::ringbuffer::ReadResultSet<std::string> > rs = f->get();

                    ASSERT_EQ(3, rs->readCount());
                    ASSERT_EQ_PTR("1", rs->getItems().get(0), std::string);
                    ASSERT_EQ_PTR("2", rs->getItems().get(1), std::string);
                    ASSERT_EQ_PTR("3", rs->getItems().get(2), std::string);
                }

                // checks if the max count works. So if more results are available than needed, the surplus results should not be read.
                TEST_F(RingbufferTest, readManyAsync_maxCount) {
                    client2Ringbuffer->add("1");
                    client2Ringbuffer->add("2");
                    client2Ringbuffer->add("3");
                    client2Ringbuffer->add("4");
                    client2Ringbuffer->add("5");
                    client2Ringbuffer->add("6");

                    boost::shared_ptr<ICompletableFuture<client::ringbuffer::ReadResultSet<std::string> > > f = clientRingbuffer->readManyAsync<void>(
                            0, 3, 3, NULL);
                    boost::shared_ptr<client::ringbuffer::ReadResultSet<std::string> > rs = f->get();

                    ASSERT_EQ(3, rs->readCount());
                    DataArray<string> &items1 = rs->getItems();
                    ASSERT_EQ_PTR("1", items1.get(0), std::string);
                    ASSERT_EQ_PTR("2", items1.get(1), std::string);
                    ASSERT_EQ_PTR("3", items1.get(2), std::string);
                }

                TEST_F(RingbufferTest, readManyAsync_withFilter) {
                    client2Ringbuffer->add("good1");
                    client2Ringbuffer->add("bad1");
                    client2Ringbuffer->add("good2");
                    client2Ringbuffer->add("bad2");
                    client2Ringbuffer->add("good3");
                    client2Ringbuffer->add("bad3");

                    StartsWithStringFilter filter("good");
                    boost::shared_ptr<ICompletableFuture<client::ringbuffer::ReadResultSet<std::string> > > f = clientRingbuffer->readManyAsync<StartsWithStringFilter>(
                            0, 3, 3, &filter);

                    boost::shared_ptr<client::ringbuffer::ReadResultSet<std::string> > rs = f->get();

                    ASSERT_EQ(5, rs->readCount());
                    DataArray<string> &items = rs->getItems();
                    ASSERT_EQ_PTR("good1", items.get(0), std::string);
                    ASSERT_EQ_PTR("good2", items.get(1), std::string);
                    ASSERT_EQ_PTR("good3", items.get(2), std::string);
                }
            }
        }
    }
}

