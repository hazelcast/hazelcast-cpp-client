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
// Created by ihsan demir on 13 Jan 2017.
//

#include <set>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMapNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<bool> {
            public:
                virtual void TearDown() {
                    g_srvFactory->shutdownAll();
                }

            protected:
                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static const std::string DEFAULT_NEAR_CACHE_NAME;

                boost::shared_ptr<config::NearCacheConfig<int, int> > newNoInvalidationNearCacheConfig() {
                    boost::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(newNearCacheConfig());
                    nearCacheConfig->setInMemoryFormat(config::OBJECT);
                    nearCacheConfig->setInvalidateOnChange(false);
                    return nearCacheConfig;
                }

                boost::shared_ptr<config::NearCacheConfig<int, int> > newNearCacheConfig() {
                    return boost::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>());
                }

                std::auto_ptr<ClientConfig> newClientConfig() {
                    return std::auto_ptr<ClientConfig>(new ClientConfig());
                }

                IMap<int, int> getNearCachedMapFromClient(
                        boost::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;
                    servers.push_back(
                            boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));

                    nearCacheConfig->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(nearCacheConfig);

                    client = std::auto_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    return client->getMap<int, int>(mapName);
                }

                monitor::NearCacheStats *getNearCacheStats(IMap<int, int> &map) {
                    return map.getLocalMapStats().getNearCacheStats();
                }

                void assertThatOwnedEntryCountEquals(IMap<int, int> &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::auto_ptr<ClientConfig> clientConfig;
                boost::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig;
                std::auto_ptr<HazelcastClient> client;
                std::vector<boost::shared_ptr<HazelcastServer> > servers;
            };

            const std::string ClientMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";

            TEST_P(ClientMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                IMap<int, int> map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::set<int> keys;

                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map.put(i, i);
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map.get(i);
                }
                // getAll() generates the Near Cache hits
                map.getAll(keys);

                monitor::NearCacheStats *stats = getNearCacheStats(map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_P(ClientMapNearCacheTest, testGetAllPopulatesNearCache) {
                IMap<int, int> map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::set<int> keys;

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map.put(i, i);
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map.get(i);
                }
                // getAll() generates the Near Cache hits
                map.getAll(keys);

                assertThatOwnedEntryCountEquals(map, size);
            }

            INSTANTIATE_TEST_CASE_P(ClientMapNearCacheTestInstance, ClientMapNearCacheTest,
                                    ::testing::Values(true, false));
        }
    }
}
