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
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::ClientConfig config;

    /**
     * Pause time between each retry cycle of an invocation in milliseconds.
     *
     * Wait only a maximum of 500 msecs between each consecutive retries. The default value is 1000msecs.
     */
    config.setProperty("hazelcast.client.invocation.retry.pause.millis", "500");

    /**
     * When an invocation gets an exception because :
     * - Member throws an exception.
     * - Connection between the client and member is closed.
     * - Client's heartbeat requests are timed out.
     * Time passed since invocation started is compared with this property.
     * If the time is already passed, then the exception is delegated to the user. If not, the invocation is retried.
     * Note that, if invocation gets no exception and it is a long running one, then it will not get any exception,
     * no matter how small this timeout is set.
     *
     * The following sets the timeout to 30 seconds. The defautl value is 120 seconds.
     */
    config.setProperty("hazelcast.client.invocation.timeout.seconds", "30");

    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");
    
    map.put(1, 100);

    std::cout << "Finished" << std::endl;

    return 0;
}
