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
// Created by İhsan Demir on Mar 6 2016.
//
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServer.h"

#include <gtest/gtest.h>
#include <boost/foreach.hpp>

#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/ILogger.h"
#include "ClientTestSupport.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace faulttolerance {
                class LoadTest : public ClientTestSupport {
                public:
                    std::auto_ptr<hazelcast::client::ClientConfig> getLoadTestConfig() {
                        std::auto_ptr<ClientConfig> config = ClientTestSupport::getConfig();
                        config->setRedoOperation(true);
                        return config;
                    }

                    class LoadClientTask : public util::Runnable {
                    public:
                        LoadClientTask(IMap<int, int> &map, int numberOfOps, util::CountDownLatch &latch) : map(map),
                                                                                                            numberOfOps(
                                                                                                                    numberOfOps),
                                                                                                            latch(latch) {}

                        virtual void run() {
                            latch.countDown();

                            latch.await(20);

                            for (int i = 0; i < numberOfOps; ++i) {
                                int mod = rand() % 3;
                                switch (mod) {
                                    case 0:
                                        ASSERT_NO_THROW(map.put(i, i));
                                        break;
                                    case 1:
                                        ASSERT_NO_THROW(map.remove(i));
                                    case 2: {
                                        boost::shared_ptr<int> val;
                                        ASSERT_NO_THROW(val = map.get(i));
                                        if ((int *) NULL != val.get()) {
                                            ASSERT_EQ(*val, i);
                                        }
                                        break;
                                    }
                                    default:
                                        abort();
                                }
                            }
                        }

                        virtual const string getName() const {
                            return "LoadClientTask";
                        }

                    private:
                        IMap<int, int> &map;
                        int numberOfOps;
                        util::CountDownLatch &latch;
                    };

                    void addThread(IMap<int, int> &map, int numberOfOps, util::CountDownLatch &latch) {
                        boost::shared_ptr<util::Runnable> task(new LoadClientTask(map, numberOfOps, latch));
                        threads.push_back(boost::shared_ptr<util::Thread>(new util::Thread(task, getLogger())));
                    }

                    void startThreads() {
                        BOOST_FOREACH(boost::shared_ptr<util::Thread> &t, threads) {
                                        t->start();
                                    }
                    }

                    void waitForThreadsToFinish() {
                        BOOST_FOREACH(boost::shared_ptr<util::Thread> &t, threads) {
                                        t->join();
                                    }
                    }

                protected:
                    std::vector<boost::shared_ptr<util::Thread> > threads;
                };

                void loadIntMapTestWithConfig(ClientConfig &config, LoadTest &test) {
                    HazelcastServer instance1(*g_srvFactory);
                    HazelcastServer instance2(*g_srvFactory);
                    HazelcastServer instance3(*g_srvFactory);
                    HazelcastServer instance4(*g_srvFactory);
                    HazelcastClient client(config);
                    IMap<int, int> imap = client.getMap<int, int>("loadtest");

                    int numThreads = 40;
                    int numOps = 1000;

                    util::CountDownLatch startLatch(numThreads);

                    for (int i = 0; i < numThreads; ++i) {
                        test.addThread(imap, numOps, startLatch);
                    }

                    test.startThreads();

                    ASSERT_TRUE(startLatch.await(20));

                    instance1.shutdown();
                    instance2.shutdown();
                    instance3.shutdown();

                    HazelcastServer instance5(*g_srvFactory);

                    /*Note: Could not shutdown instance 5 here, since there may be some incomplete synchronization
                     * between instance 5 and instance 4. This caused problems in Linux environment. */

                    test.waitForThreadsToFinish();

                }

                TEST_F(LoadTest, testIntMapSmartClientServerRestart) {
                    std::auto_ptr<ClientConfig> config = getLoadTestConfig();
                    config->setSmart(true);

                    loadIntMapTestWithConfig(*config, *this);
                }

                TEST_F(LoadTest, testIntMapDummyClientServerRestart) {
                    std::auto_ptr<ClientConfig> config = getLoadTestConfig();
                    config->setSmart(false);

                    loadIntMapTestWithConfig(*config, *this);
                }
            }
        }
    }
}
