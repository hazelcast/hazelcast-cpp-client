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
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <set>
#include <memory>
#include <hazelcast/client/query/EqualPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapNearCacheTest : public ClientTestSupport {
            protected:
                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static const std::string DEFAULT_NEAR_CACHE_NAME;

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance2;
                    delete instance;
                    instance2 = NULL;
                    instance = NULL;
                }


                virtual void TearDown() {
                    if (map.get()) {
                        map->destroy();
                    }
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNoInvalidationNearCacheConfig() {
                    std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(newNearCacheConfig());
                    nearCacheConfig->setInMemoryFormat(config::OBJECT);
                    nearCacheConfig->setInvalidateOnChange(false);
                    return nearCacheConfig;
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNearCacheConfig() {
                    return std::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>());
                }

                std::unique_ptr<ClientConfig> newClientConfig() {
                    return std::unique_ptr<ClientConfig>(new ClientConfig());
                }

                std::shared_ptr<ReplicatedMap<int, int> > getNearCachedMapFromClient(
                        std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    nearCacheConfig->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(nearCacheConfig);

                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    map = client->getReplicatedMap<int, int>(mapName);
                    return map;
                }

                monitor::NearCacheStats *getNearCacheStats(ReplicatedMap<int, int> &map) {
                    return  (static_cast<proxy::ClientReplicatedMapProxy<int, int> &>(map)).getNearCacheStats();
                }

                void assertThatOwnedEntryCountEquals(ReplicatedMap<int, int> &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::unique_ptr<ClientConfig> clientConfig;
                std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::shared_ptr<ReplicatedMap<int, int> > map;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const std::string ClientReplicatedMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance = NULL;
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance2 = NULL;

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }
                // getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                monitor::NearCacheStats *stats = getNearCacheStats(*map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllPopulatesNearCache) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }
                // getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                assertThatOwnedEntryCountEquals(*map, size);
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testRemoveAllNearCache) {
                std::shared_ptr<ReplicatedMap<int, int> > map = getNearCachedMapFromClient(newNearCacheConfig());


                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get(i);
                }

                for (int i = 0; i < size; i++) {
                    map->remove(i);
                }

                assertThatOwnedEntryCountEquals(*map, 0);
            }
        }
    }
}
