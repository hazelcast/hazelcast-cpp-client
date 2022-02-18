/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <benchmark/benchmark.h>
#include <hazelcast/client/hazelcast_client.h>

using namespace hazelcast::client;

static auto client{ hazelcast::new_client().get() };
static auto bm_map = client.get_map("map_put").get();

static void
map_put(benchmark::State& state)
{
    int number_of_puts = 0;
    for (auto _ : state) {
        auto key = rand() % 10000;
        bm_map->put(key, key).get();
        state.counters["Put Rate"] =
          benchmark::Counter(++number_of_puts, benchmark::Counter::kIsRate);
    }
}

static void
map_get(benchmark::State& state)
{
    int number_of_puts = 0;
    for (auto _ : state) {
        auto key = rand() % 10000;
        bm_map->get<int, int>(key).get();
        state.counters["Get Rate"] =
          benchmark::Counter(++number_of_puts, benchmark::Counter::kIsRate);
    }
}

static void
map_remove(benchmark::State& state)
{
    int number_of_puts = 0;
    for (auto _ : state) {
        auto key = rand() % 10000;
        bm_map->remove<int, int>(key).get();
        state.counters["Remove Rate"] =
          benchmark::Counter(++number_of_puts, benchmark::Counter::kIsRate);
    }
}

BENCHMARK(map_put)->Threads(32);
BENCHMARK(map_get)->Threads(32);
BENCHMARK(map_remove)->Threads(32);

BENCHMARK_MAIN();
