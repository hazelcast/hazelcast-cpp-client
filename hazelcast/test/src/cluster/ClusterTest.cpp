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
// Created by sancar koyunlu on 26/02/14.
//
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServer.h"

#include <gtest/gtest.h>
#include "ClientTestSupportBase.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClusterTest : public ClientTestSupportBase, public ::testing::TestWithParam<ClientConfig *> {
            public:
                ClusterTest() : sslFactory(getSslFilePath()) {}

            protected:
                class ClientAllStatesListener : public LifecycleListener {
                public:

                    ClientAllStatesListener(util::CountDownLatch *startingLatch,
                                            util::CountDownLatch *startedLatch = NULL,
                                            util::CountDownLatch *connectedLatch = NULL,
                                            util::CountDownLatch *disconnectedLatch = NULL,
                                            util::CountDownLatch *shuttingDownLatch = NULL,
                                            util::CountDownLatch *shutdownLatch = NULL)
                            : startingLatch(startingLatch), startedLatch(startedLatch), connectedLatch(connectedLatch),
                              disconnectedLatch(disconnectedLatch), shuttingDownLatch(shuttingDownLatch),
                              shutdownLatch(shutdownLatch) {}

                    virtual void stateChanged(const LifecycleEvent &lifecycleEvent) {
                        switch (lifecycleEvent.getState()) {
                            case LifecycleEvent::STARTING:
                                if (startingLatch) {
                                    startingLatch->countDown();
                                }
                                break;
                            case LifecycleEvent::STARTED:
                                if (startedLatch) {
                                    startedLatch->countDown();
                                }
                                break;
                            case LifecycleEvent::CLIENT_CONNECTED:
                                if (connectedLatch) {
                                    connectedLatch->countDown();
                                }
                                break;
                            case LifecycleEvent::CLIENT_DISCONNECTED:
                                if (disconnectedLatch) {
                                    disconnectedLatch->countDown();
                                }
                                break;
                            case LifecycleEvent::SHUTTING_DOWN:
                                if (shuttingDownLatch) {
                                    shuttingDownLatch->countDown();
                                }
                                break;
                            case LifecycleEvent::SHUTDOWN:
                                if (shutdownLatch) {
                                    shutdownLatch->countDown();
                                }
                                break;
                            default:
                                FAIL() << "No such state expected:" << lifecycleEvent.getState();
                        }
                    }

                private:
                    util::CountDownLatch *startingLatch;
                    util::CountDownLatch *startedLatch;
                    util::CountDownLatch *connectedLatch;
                    util::CountDownLatch *disconnectedLatch;
                    util::CountDownLatch *shuttingDownLatch;
                    util::CountDownLatch *shutdownLatch;
                };

                std::auto_ptr<HazelcastServer> startServer(ClientConfig &clientConfig) {
                    if (clientConfig.getNetworkConfig().getSSLConfig().isEnabled()) {
                        return std::auto_ptr<HazelcastServer>(new HazelcastServer(sslFactory));
                    } else {
                        return std::auto_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory));
                    }
                }

            private:
                HazelcastServerFactory sslFactory;
            };

            class SmartTcpClientConfig : public ClientConfig {
            };

            class SmartSSLClientConfig : public ClientConfig {
            public:
                SmartSSLClientConfig() {
                    this->getNetworkConfig().getSSLConfig().setEnabled(true);
                }
            };

            TEST_P(ClusterTest, testBehaviourWhenClusterNotFound) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testDummyClientBehaviourWhenClusterNotFound) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.setSmart(false);
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

            TEST_P(ClusterTest, testAllClientStates) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());

                std::auto_ptr<HazelcastServer> instance = startServer(clientConfig);

                clientConfig.setAttemptPeriod(1000);
                clientConfig.setConnectionAttemptLimit(1);
                util::CountDownLatch startingLatch(1);
                util::CountDownLatch startedLatch(1);
                util::CountDownLatch connectedLatch(1);
                util::CountDownLatch disconnectedLatch(1);
                util::CountDownLatch shuttingDownLatch(1);
                util::CountDownLatch shutdownLatch(1);
                ClientAllStatesListener listener(&startingLatch, &startedLatch, &connectedLatch, &disconnectedLatch,
                                                 &shuttingDownLatch, &shutdownLatch);
                clientConfig.addListener(&listener);

                HazelcastClient client(clientConfig);

                ASSERT_TRUE(startingLatch.await(0));
                ASSERT_TRUE(startedLatch.await(0));
                ASSERT_TRUE(connectedLatch.await(0));

                instance->shutdown();

                ASSERT_OPEN_EVENTUALLY(disconnectedLatch);
                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

            TEST_P(ClusterTest, testConnectionAttemptPeriod) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                clientConfig.getNetworkConfig().setConnectionAttemptPeriod(900).
                        setConnectionTimeout(2000).setConnectionAttemptLimit(2);
                clientConfig.getNetworkConfig().addAddress(Address("8.8.8.8", 8000));

                int64_t startTimeMillis = util::currentTimeMillis();
                try {
                    HazelcastClient client(clientConfig);
                } catch (exception::IllegalStateException &) {
                    // this is expected
                }
                ASSERT_GE(util::currentTimeMillis() - startTimeMillis, 2 * 900);
            }

            TEST_P(ClusterTest, testAllClientStatesWhenUserShutdown) {
                ClientConfig &clientConfig = *const_cast<ParamType &>(GetParam());
                std::auto_ptr<HazelcastServer> instance = startServer(clientConfig);

                util::CountDownLatch startingLatch(1);
                util::CountDownLatch startedLatch(1);
                util::CountDownLatch connectedLatch(1);
                util::CountDownLatch disconnectedLatch(1);
                util::CountDownLatch shuttingDownLatch(1);
                util::CountDownLatch shutdownLatch(1);
                ClientAllStatesListener listener(&startingLatch, &startedLatch, &connectedLatch, &disconnectedLatch,
                                                 &shuttingDownLatch, &shutdownLatch);
                clientConfig.addListener(&listener);

                HazelcastClient client(clientConfig);

                ASSERT_TRUE(startingLatch.await(0));
                ASSERT_TRUE(startedLatch.await(0));
                ASSERT_TRUE(connectedLatch.await(0));

                client.shutdown();

                ASSERT_OPEN_EVENTUALLY(shuttingDownLatch);
                ASSERT_OPEN_EVENTUALLY(shutdownLatch);
            }

            #ifdef HZ_BUILD_WITH_SSL

            INSTANTIATE_TEST_CASE_P(All,
                                    ClusterTest,
                                    ::testing::Values(new SmartTcpClientConfig(), new SmartSSLClientConfig()));
            #else
            INSTANTIATE_TEST_CASE_P(All,
                                    ClusterTest,
                                    ::testing::Values(new SmartTcpClientConfig()));
            #endif
        }
    }
}


