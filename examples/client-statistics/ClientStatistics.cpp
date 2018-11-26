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
#include <stdint.h>

#include <hazelcast/client/HazelcastClient.h>

using namespace hazelcast::client;

/**
 * Important note: Please make sure a cluster is running and management center is enabled at the members and url's
 * point to the management center url where the management center is started. See the example configuration file
 * `hazelcast-management-center-enabled.xml` for member configuration.
 *
 */
int main() {
    ClientConfig config;

    config.setProperty(ClientProperties::STATISTICS_ENABLED, "true");

    /**
     * Collect and send statistics every 5 seconds
     */
    config.setProperty(ClientProperties::STATISTICS_PERIOD_SECONDS, "5");

    config.addNearCacheConfig(boost::shared_ptr<config::NearCacheConfig<int, int> >(new config::NearCacheConfig<int, int>("MyMap")));
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::IMap<int, int> map = hz.getMap<int, int>("MyMap");
    
    map.put(2, 500);

    // generate a near-cache miss
    map.get(2);

    // generate two near-cache hits
    map.get(2);
    map.get(2);

    // sleep more than the statistics collection time and keep the client running. Statistics is now populated at the
    // member side, so you can see them at the Management Center.
    hazelcast::util::sleep(100);

    std::cout << "Finished" << std::endl;

    return 0;
}
