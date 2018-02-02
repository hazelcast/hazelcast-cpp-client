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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
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

            IssueTest::IssueTest()
            : latch(2), listener(latch) {
            }

            IssueTest::~IssueTest() {
            }
            
            void threadTerminateNode(util::ThreadArgs &args) {
                HazelcastServer *node = (HazelcastServer *) args.arg0;
                node->shutdown();
            }

            void putMapMessage(util::ThreadArgs &args) {
                IMap<int, int> *map = (IMap<int, int> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;

                do {
                    // 7. Put a 2nd entry to the map
                    try {
                        map->put(2, 20);
                    } catch (std::exception &e) {
                        // suppress the error
						(void)e; // suppress the unused variable warning
                    }
                    util::sleep(1);
                } while (latch->get() > 0);

            }

            TEST_F(IssueTest, testOperationRedo_smartRoutingDisabled) {
                HazelcastServer hz1(*g_srvFactory);
                HazelcastServer hz2(*g_srvFactory);

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->setRedoOperation(true);
                clientConfig->setSmart(false);

                HazelcastClient client(*clientConfig);

                client::IMap<int, int> map = client.getMap<int, int>("m");
                util::Thread* thread = NULL;
                int expected = 1000;
                for (int i = 0; i < expected; i++) {
                    if(i == 5){
                        thread = new util::Thread(threadTerminateNode, &hz1);
                    }
                    map.put(i, i);
                }
                thread->join();
                delete thread;
                ASSERT_EQ(expected, map.size());
            }

            TEST_F(IssueTest, testListenerSubscriptionOnSingleServerRestart) {
                HazelcastServer server(*g_srvFactory);

                // 2. Start a client
                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->setConnectionAttemptLimit(10);

                HazelcastClient client(*clientConfig);

                // 3. Get a map
                IMap <int, int> map = client.getMap<int, int>("IssueTest_map");

                // 4. Subscribe client to entry added event
                map.addEntryListener(listener, true);

                // Put a key, value to the map
                ASSERT_EQ((int *)NULL, map.put(1, 10).get());

                ASSERT_TRUE(latch.await(20, 1)); // timeout of 20 seconds

                // 5. Verify that the listener got the entry added event
                ASSERT_EQ(1, latch.get());

                // 6. Restart the server
                ASSERT_TRUE(server.shutdown());
                ASSERT_TRUE(server.start());

                std::string putThreadName("Map Put Thread");
                util::Thread t(putThreadName, putMapMessage, &map, &latch);

                // 8. Verify that the 2nd entry is received by the listener
                ASSERT_TRUE(latch.await(20, 0)); // timeout of 20 seconds

                t.cancel();
                t.join();

                // 9. Shut down the server
                ASSERT_TRUE(server.shutdown());
            }

            TEST_F(IssueTest, testIssue221) {
                // start a server
                HazelcastServer server(*g_srvFactory);
                
                // start a client
                std::auto_ptr<ClientConfig> config = getConfig();
                HazelcastClient client(*config);

                IMap<int, int> map = client.getMap<int, int>("Issue221_test_map");

                server.shutdown();

                try {
                    map.get(1);
                } catch (exception::IOException &) {
                    // this is the expected exception, test passes, do nothing
                } catch (exception::IException &e) {
                    std::string msg = e.what();
                    if (msg.find("ConnectionManager is not active") == std::string::npos) {
                        FAIL() << "Unexpected exception. Received exception:" << msg;
                    }
                }
            }

            void IssueTest::Issue864MapListener::entryAdded(const EntryEvent<int, int> &event) {
                int count = latch.get();
                if (2 == count) {
                    // The received event should be the addition of key value: 1, 10
                    ASSERT_EQ(1, event.getKey());
                    ASSERT_EQ(10, event.getValue());
                } else if (1 == count) {
                    // The received event should be the addition of key value: 2, 20
                    ASSERT_EQ(2, event.getKey());
                    ASSERT_EQ(20, event.getValue());
                }

                latch.countDown();
            }

            void IssueTest::Issue864MapListener::entryUpdated(const EntryEvent<int, int> &event) {
                ASSERT_EQ(2, event.getKey());
                ASSERT_EQ(20, event.getValue());
                latch.countDown();
            }

            IssueTest::Issue864MapListener::Issue864MapListener(util::CountDownLatch &l) : latch(l) {

            }
        }
    }
}
