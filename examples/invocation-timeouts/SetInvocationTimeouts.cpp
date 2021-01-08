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
#include <hazelcast/client/hazelcast_client.h>

int main() {
    hazelcast::client::client_config config;

    /**
     * Pause time between each retry cycle of an invocation in milliseconds.
     *
     * Wait only a maximum of 500 msecs between each consecutive retries. The default value is 1000msecs.
     */
    config.set_property("hazelcast.client.invocation.retry.pause.millis", "500");

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
     * The following sets the timeout to 30 seconds. The default value is 120 seconds.
     */
    config.set_property("hazelcast.client.invocation.timeout.seconds", "30");

    hazelcast::client::hazelcast_client hz(std::move(config));
    hz.start().get();

    auto map = hz.get_map("MyMap").get();
    
    map->put(1, 100).get();

    // while doing the map->get in the following loop just shutdown the cluster and restart within 30 seconds, you will
    // observe that the retries will be done and it will finally succeed when the cluster comes back again.
    for (int i=0;i < 100000; ++i) {
        map->get<int, int>(1).get();
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
