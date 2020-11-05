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
#include <random>

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/Pipelining.h>

using namespace std;
using namespace hazelcast::client;
using namespace hazelcast::util;

/**
 * A demonstration of the performance impact of using pipeling.
 *
 * For the benchmark we compare simple IMap::get calls with a pipelined approach.
 */

class PipeliningDemo {
public:
    PipeliningDemo() : client_(clientConfig_), map_(client_.getMap("map")), gen_(rd_()) {}

    void init() {
        for (int l = 0; l < keyDomain; l++) {
            map_->put(l, std::to_string(l)).get();
        }
    }

    void pipelined(int depth) {
        cout << "Starting pipelined with depth:" << depth << endl;
        int64_t startMs = currentTimeMillis();
        for (int i = 0; i < iterations; i++) {
            std::shared_ptr<Pipelining<string> > pipelining = Pipelining<string>::create(depth);
            for (long k = 0; k < getsPerIteration; k++) {
                int key = dist_(gen_) % keyDomain;
                pipelining->add(map_->get<int, std::string>(key));
            }

            // wait for completion
            auto results = pipelining->results();
            // and verification we got the appropriate number of results.
            if ((int) results.size() != getsPerIteration) {
                throw hazelcast::client::exception::IllegalStateException("pipelined", "Incorrect number of results");
            }
        }
        int64_t endMs = currentTimeMillis();
        cout << "Pipelined with depth:" << depth << ", duration:" << (endMs - startMs) << " ms" << endl;
    }

    void nonPipelined() {
        cout << "Starting non pipelined" << endl;
        int64_t startMs = currentTimeMillis();
        for (int i = 0; i < iterations; i++) {
            for (long k = 0; k < getsPerIteration; k++) {
                int key = dist_(gen_) % keyDomain;
                map_->get<int, std::string>(key).get();
            }
        }
        int64_t endMs = currentTimeMillis();
        cout << "Non pipelined duration:" << (endMs - startMs) << " ms" << endl;
    }

private:
    HazelcastClient client_;
    std::shared_ptr<IMap> map_;
    ClientConfig clientConfig_;
    static const int keyDomain = 100000;
    static const int iterations = 500;
    static const int getsPerIteration = 1000;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<int> dist_;
};

int main() {
    PipeliningDemo main;
    main.init();
    main.pipelined(5);
    main.pipelined(10);
    main.pipelined(100);
    main.nonPipelined();

    cout << "Finished" << endl;

    return 0;
}
