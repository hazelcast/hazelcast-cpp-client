/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/LifecycleListener.h>

class ConnectedListener : public hazelcast::client::LifecycleListener {
public:
    ConnectedListener() : latch(1) {}

    virtual void stateChanged(const hazelcast::client::LifecycleEvent &lifecycleEvent) {
        if (lifecycleEvent.getState() == hazelcast::client::LifecycleEvent::CLIENT_CONNECTED) {
            latch.countDown();
        }
    }

    bool isConnected() {
        return latch.get() == 0;
    }

    void waitForConnection() {
        latch.await();
    }
private:
    hazelcast::util::CountDownLatch latch;
};

int main() {
    ConnectedListener listener;

    hazelcast::client::ClientConfig config;

    /**
     * Set true for non blocking {@link HazelcastClient(const ClientConfig &)}. The client creation won't wait to
     * connect to cluster. The client instace will throw exception until it connects to cluster and become ready.
     * If set to false, {@link HazelcastClient(const ClientConfig &)} will block until a cluster connection established and it's
     * ready to use client instance.
     *
     * default value is false
     */
    config.getConnectionStrategyConfig().setAsyncStart(true);

    // Added a lifecycle listener so that we can track when the client is connected
    config.addListener(&listener);

    hazelcast::client::HazelcastClient hz(config);

    // the client may not have connected to the cluster yet at this point since the cluster connection is async!!!
    if (!listener.isConnected()) {
        std::cout << "Async client is not connected yet." << std::endl;
    }

    listener.waitForConnection();

    std::cout << "Async client is connected now." << std::endl;

    std::cout << "Finished" << std::endl;

    hz.shutdown();

    return 0;
}

