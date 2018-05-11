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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/LifecycleListener.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace connectionstrategy {

                class ConfiguredBehaviourTest : public ClientTestSupport {
                public:
                    ConfiguredBehaviourTest() {
                        clientConfig.getNetworkConfig().setConnectionTimeout(2000).setConnectionAttemptLimit(2).
                                setConnectionAttemptPeriod(1000);
                    }

                protected:
                    class LifecycleStateListener : public LifecycleListener {
                    public:
                        LifecycleStateListener(util::CountDownLatch &connectedLatch,
                                               const LifecycleEvent::LifeCycleState expectedState)
                                : connectedLatch(connectedLatch), expectedState(expectedState) {}

                        virtual void stateChanged(const LifecycleEvent &event) {
                            if (event.getState() == expectedState) {
                                connectedLatch.countDown();
                            }
                        }

                    private:
                        util::CountDownLatch &connectedLatch;
                        const LifecycleEvent::LifeCycleState expectedState;
                    };
                    
                    ClientConfig clientConfig;
                };

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster) {
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);
                    HazelcastClient client(clientConfig);

                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    /* TODO: Change to HazelcastClientOfflineException after https://github.com/hazelcast/hazelcast-cpp-client/issues/54 is solved*/
                    ASSERT_THROW(map.size(), exception::HazelcastClientNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster_thenShutdown) {
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);
                    HazelcastClient client(clientConfig);
                    client.shutdown();
                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    ASSERT_THROW(map.size(), exception::HazelcastClientNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrue) {
                    util::CountDownLatch connectedLatch(1);

                    // trying 8.8.8.8 address will delay the initial connection since no such server exist
                    clientConfig.getNetworkConfig().addAddress(Address("8.8.8.8", 5701))
                            .addAddress(Address("127.0.0.1", 5701));
                    clientConfig.setProperty("hazelcast.client.shuffle.member.list", "false");
                    LifecycleStateListener lifecycleListener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&lifecycleListener);
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);

                    HazelcastClient client(clientConfig);

                    ASSERT_TRUE(client.getLifecycleService().isRunning());

                    HazelcastServer server(*g_srvFactory);

                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.size();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    util::CountDownLatch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

                    // no exception at this point
                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.put(1, 5);

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map.put(1, 5), exception::HazelcastInstanceNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFTwoMembers) {
                    HazelcastServer ownerServer(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    HazelcastServer hazelcastInstance2(*g_srvFactory);
                    util::CountDownLatch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

                    // no exception at this point
                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.put(1, 5);

                    ownerServer.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map.put(1, 5), exception::HazelcastInstanceNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberInitiallyOffline) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    util::CountDownLatch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

                    // no exception at this point
                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.put(1, 5);

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map.put(1, 5), exception::HazelcastInstanceNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    util::CountDownLatch connectedLatch(1);

                    LifecycleStateListener listener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);

                    assertTrue(client.getLifecycleService().isRunning());

                    assertOpenEventually(connectedLatch);

                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.size();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberStartLate) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    util::CountDownLatch initialConnectionLatch(1);
                    util::CountDownLatch reconnectedLatch(1);

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(10);
                    LifecycleStateListener listener(initialConnectionLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);

                    hazelcastInstance.shutdown();

                    LifecycleStateListener reconnectListener(reconnectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    client.addLifecycleListener(&reconnectListener);

                    HazelcastServer hazelcastInstance2(*g_srvFactory);

                    assertTrue(client.getLifecycleService().isRunning());
                    assertOpenEventually(reconnectedLatch);

                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.size();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCTwoMembers) {
                    HazelcastServer ownerServer(*g_srvFactory);

                    util::CountDownLatch connectedLatch(1);
                    util::CountDownLatch disconnectedLatch(1);
                    util::CountDownLatch reconnectedLatch(1);

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(10);
                    LifecycleStateListener listener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);

                    assertTrue(client.getLifecycleService().isRunning());

                    assertOpenEventually(connectedLatch);

                    HazelcastServer hazelcastInstance2(*g_srvFactory);

                    IMap<int, int> map = client.getMap<int, int>(randomMapName());
                    map.put(1, 5);

                    LifecycleStateListener disconnectListener(disconnectedLatch, LifecycleEvent::CLIENT_DISCONNECTED);
                    client.addLifecycleListener(&disconnectListener);

                    LifecycleStateListener reconnectListener(reconnectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    client.addLifecycleListener(&reconnectListener);

                    ownerServer.shutdown();

                    assertOpenEventually(disconnectedLatch);
                    assertOpenEventually(reconnectedLatch);

                    map.get(1);

                    client.shutdown();
                }
            }
        }
    }
}

