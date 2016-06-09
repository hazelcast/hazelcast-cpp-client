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
// Created by ihsan demir on 06 June 2016.

#include "hazelcast/client/ReliableTopic.h"
#include "hazelcast/client/HazelcastClient.h"

#include "../HazelcastServerFactory.h"
#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"
#include "../serialization/Employee.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ReliableTopicTest : public ClientTestSupport {
            protected:
                template <typename T>
                class GenericListener : public topic::ReliableMessageListener<T> {
                public:
                    GenericListener(util::CountDownLatch &countDownLatch) : latch(countDownLatch), startSequence(-1),
                                                                               numberOfMessagesReceived(0) {
                    }

                    GenericListener(util::CountDownLatch &countDownLatch, int64_t sequence) : latch(countDownLatch),
                                                                                                 startSequence(sequence),
                                                                                                 numberOfMessagesReceived(0) {
                    }

                    virtual ~GenericListener() {
                        T *e = NULL;
                        while ((e = objects.poll()) != NULL) {
                            delete(e);
                        }
                    }

                    virtual void onMessage(std::auto_ptr<topic::Message<T> > message) {
                        ++numberOfMessagesReceived;

                        const T *object = message->getMessageObject();
                        std::ostringstream out;
                        if (NULL != object) {
                            out << "[GenericListener::onMessage] Received message: " << message->getMessageObject() <<
                            " for topic:" << message->getName();
                        } else {
                            out << "[GenericListener::onMessage] Received message with NULL object for topic:" << message->getName();
                        }
                        util::ILogger::getLogger().info(out.str());

                        objects.offer(message->releaseMessageObject().release());
                        latch.countDown();
                    }

                    virtual int64_t retrieveInitialSequence() const {
                        return startSequence;
                    }

                    virtual void storeSequence(int64_t sequence) {

                    }

                    virtual bool isLossTolerant() const {
                        return false;
                    }

                    virtual bool isTerminal(const exception::IException &failure) const {
                        return false;
                    }

                    int getNumberOfMessagesReceived() {
                        int value = numberOfMessagesReceived;
                        return value;
                    }

                    util::ConcurrentQueue<T> &getObjects() {
                        return objects;
                    }
                private:
                    util::CountDownLatch &latch;
                    int64_t startSequence;
                    util::AtomicInt numberOfMessagesReceived;
                    util::ConcurrentQueue<T> objects;
                };

                class IntListener : public GenericListener<int> {
                public:
                    IntListener(util::CountDownLatch &countDownLatch) : GenericListener<int>(countDownLatch) { }

                    IntListener(util::CountDownLatch &countDownLatch, int64_t sequence) : GenericListener<int>(
                            countDownLatch, sequence) { }
                };

                class MyReliableListener : public GenericListener<Employee> {
                public:
                    MyReliableListener(util::CountDownLatch &countDownLatch, int64_t sequence) : GenericListener<Employee>(
                            countDownLatch, sequence) { }

                    MyReliableListener(util::CountDownLatch &countDownLatch) : GenericListener<Employee>(countDownLatch) { }
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
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
            };

            HazelcastServer *ReliableTopicTest::instance = NULL;
            ClientConfig *ReliableTopicTest::clientConfig = NULL;
            HazelcastClient *ReliableTopicTest::client = NULL;

            TEST_F(ReliableTopicTest, testBasics) {
                std::auto_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("testBasics"));

                ASSERT_EQ("testBasics", rt->getName());

                util::CountDownLatch latch(1);

                MyReliableListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                Employee empl1("first", 20);

                ASSERT_NO_THROW(rt->publish(&empl1));

                ASSERT_TRUE(latch.await(2));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                Employee *employee = listener.getObjects().poll();
                ASSERT_NE((Employee *)NULL, employee);
                ASSERT_EQ(empl1, *employee);

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));
                ASSERT_FALSE(rt->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, testListenerSequence) {
                std::auto_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("testListenerSequence"));

                Employee empl1("first", 10);
                Employee empl2("second", 20);

                ASSERT_NO_THROW(rt->publish(&empl1));
                ASSERT_NO_THROW(rt->publish(&empl2));

                util::CountDownLatch latch(1);

                MyReliableListener listener(latch, 1);
                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                ASSERT_TRUE(latch.await(1));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                Employee *employee = listener.getObjects().poll();
                ASSERT_NE((Employee *)NULL, employee);
                ASSERT_EQ(empl2, *employee);

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenExisting) {
                std::auto_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("removeMessageListener_whenExisting"));

                Employee empl1("first", 10);

                util::CountDownLatch latch(1);

                MyReliableListener listener(latch);
                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));

                ASSERT_NO_THROW(rt->publish(&empl1));

                ASSERT_FALSE(latch.await(2));
                ASSERT_EQ(0, listener.getNumberOfMessagesReceived());
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenNonExisting) {
                std::auto_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("removeMessageListener_whenNonExisting"));

                // remove listener
                ASSERT_FALSE(rt->removeMessageListener("abc"));
            }

            TEST_F(ReliableTopicTest, publishNull) {
                std::auto_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("publishNull"));

                util::CountDownLatch latch(1);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                intTopic->publish((int *)NULL);

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                int *val = listener.getObjects().poll();
                ASSERT_EQ((int *)NULL, val);
            }
        }
    }
}

