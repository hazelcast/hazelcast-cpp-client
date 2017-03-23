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
// Created by ihsan demir on 06 June 2016.

#include "hazelcast/client/ReliableTopic.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/ConcurrentQueue.h"

#include "../HazelcastServerFactory.h"
#include "../ClientTestSupport.h"
#include "../HazelcastServer.h"
#include "../serialization/Employee.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ReliableTopicTest : public ClientTestSupport {
            public:
                static void publishTopics(util::ThreadArgs &args) {
                    ReliableTopic<int> *topic = (ReliableTopic<int> *)args.arg0;
                    std::vector<int> *publishValues = (std::vector<int> *)args.arg1;

                    util::sleep(5);

                    for (std::vector<int>::const_iterator it = publishValues->begin();it != publishValues->end(); ++it) {
                        topic->publish(&(*it));
                    }
                }
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
                        topic::Message<T> *m = NULL;
                        while ((m = messages.poll()) != NULL) {
                            delete(m);
                        }
                    }

                    virtual void onMessage(std::auto_ptr<topic::Message<T> > message) {
                        ++numberOfMessagesReceived;

                        const T *object = message->getMessageObject();
                        std::ostringstream out;
                        if (NULL != object) {
                            out << "[GenericListener::onMessage] Received message: " << *message->getMessageObject() <<
                            " for topic:" << message->getName();
                        } else {
                            out << "[GenericListener::onMessage] Received message with NULL object for topic:" << message->getName();
                        }
                        util::ILogger::getLogger().info(out.str());

                        objects.offer(message->releaseMessageObject().release());
                        latch.countDown();

                        messages.offer(message.release());
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

                    util::ConcurrentQueue<topic::Message<T> > &getMessages() {
                        return messages;
                    }
                private:
                    util::CountDownLatch &latch;
                    int64_t startSequence;
                    util::AtomicInt numberOfMessagesReceived;
                    util::ConcurrentQueue<T> objects;
                    util::ConcurrentQueue<topic::Message<T> > messages;
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
                boost::shared_ptr<ReliableTopic<Employee> > rt;
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
                boost::shared_ptr<ReliableTopic<Employee> > rt;
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
                boost::shared_ptr<ReliableTopic<Employee> > rt;
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
                boost::shared_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("removeMessageListener_whenNonExisting"));

                // remove listener
                ASSERT_FALSE(rt->removeMessageListener("abc"));
            }

            TEST_F(ReliableTopicTest, publishNull) {
                boost::shared_ptr<ReliableTopic<int> > intTopic;
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

            TEST_F(ReliableTopicTest, publishMultiple) {
                boost::shared_ptr<ReliableTopic<std::string> > topic;
                ASSERT_NO_THROW(topic = client->getReliableTopic<std::string>("publishMultiple"));

                util::CountDownLatch latch(5);
                GenericListener<std::string> listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(listener));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = util::IOUtil::to_string<int>(k);
                    topic->publish(&item);
                    items.push_back(item);
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(5, listener.getNumberOfMessagesReceived());
                util::ConcurrentQueue<std::string> &queue = listener.getObjects();
                for (int k = 0; k < 5; k++) {
                    std::string *val = queue.poll();
                    ASSERT_NE((std::string *)NULL, val);
                    ASSERT_EQ(items[k], *val);
                }
            }

            TEST_F(ReliableTopicTest, testConfig) {
                ClientConfig clientConfig;
                clientConfig.addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                config::ReliableTopicConfig relConfig("testConfig");
                relConfig.setReadBatchSize(2);
                clientConfig.addReliableTopicConfig(relConfig);
                HazelcastClient configClient(clientConfig);

                boost::shared_ptr<ReliableTopic<std::string> > topic;
                ASSERT_NO_THROW(topic = configClient.getReliableTopic<std::string>("testConfig"));

                util::CountDownLatch latch(5);
                GenericListener<std::string> listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(listener));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = util::IOUtil::to_string<int>(k);
                    topic->publish(&item);
                    items.push_back(item);
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(5, listener.getNumberOfMessagesReceived());
                util::ConcurrentQueue<std::string> &queue = listener.getObjects();
                for (int k = 0; k < 5; k++) {
                    std::string *val = queue.poll();
                    ASSERT_NE((std::string *)NULL, val);
                    ASSERT_EQ(items[k], *val);
                }
            }

            TEST_F(ReliableTopicTest, testMessageFieldSetCorrectly) {
                boost::shared_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("testMessageFieldSetCorrectly"));

                util::CountDownLatch latch(1);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                int64_t timeBeforePublish = util::currentTimeMillis();
                int publishedValue = 3;
                intTopic->publish(&publishedValue);
                int64_t timeAfterPublish = util::currentTimeMillis();

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                int *val = listener.getObjects().poll();
                ASSERT_EQ(publishedValue, *val);

                topic::Message<int> *message = listener.getMessages().poll();
                ASSERT_LE(timeBeforePublish, message->getPublishTime());
                ASSERT_GE(timeAfterPublish, message->getPublishTime());
                ASSERT_EQ(intTopic->getName(), message->getSource());
                ASSERT_EQ((Member *)NULL, message->getPublishingMember());
            }

            // makes sure that when a listener is register, we don't see any messages being published before
            // it got registered. We'll only see the messages after it got registered.
            TEST_F(ReliableTopicTest, testAlwaysStartAfterTail) {
                boost::shared_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("testAlwaysStartAfterTail"));

                int publishedValue = 1;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));
                publishedValue = 2;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));
                publishedValue = 3;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));

                std::vector<int> expectedValues;
                expectedValues.push_back(4);
                expectedValues.push_back(5);
                expectedValues.push_back(6);

                // spawn a thread for publishing new data
                util::Thread t(publishTopics, intTopic.get(), &expectedValues);

                util::CountDownLatch latch(3);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ((int)expectedValues.size(), listener.getNumberOfMessagesReceived());
                util::ConcurrentQueue<int> &objects = listener.getObjects();

                for (std::vector<int>::const_iterator it = expectedValues.begin();it != expectedValues.end(); ++it) {
                    int *val = objects.poll();
                    ASSERT_NE((int *)NULL, val);
                    ASSERT_EQ(*it, *val);
                }
            }
        }
    }
}

