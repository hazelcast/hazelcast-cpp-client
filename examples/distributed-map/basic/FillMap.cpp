/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/pipelining.h"

struct HZClient {
    std::shared_ptr<hazelcast::client::hazelcast_client> client_;
};

template <typename K, typename V>
class TMap : public std::unordered_map<K, V> {
};

std::vector<HZClient> clientPool_;

auto writeChunkSize_ = 10000;

auto depth_ = 1000;

auto mapName_ = "P1Stressed";

std::mutex putMutex_;

// Share TMap, HZClient code.

template <typename K, typename V>
long put(const TMap <K, V> &tMap) {
    using namespace hazelcast::client;

    long recordsWritten = 0;

    auto &client = clientPool_[0];

    try {

        std::vector<std::shared_ptr<TMap < K, V>> > xTmpMaps;

        std::shared_ptr<imap> hzMap = client.client_->get_map(mapName_).get();

        long numRecords = tMap.size();

        long totChunks = numRecords / writeChunkSize_;

        long writtenChunks = 0;

        if (numRecords % writeChunkSize_ > 0)
            ++totChunks;

        long i = 0;

        typename std::vector<std::shared_ptr<TMap < K, V>> > ::iterator xItr = xTmpMaps.emplace(xTmpMaps.end(), std::make_shared<TMap < K, V>>());

        std::lock_guard<std::mutex> lock(putMutex_);
        {

            for (auto &item: tMap) {

                ++i;

                (*xItr)->emplace(item);

                if (i % writeChunkSize_ == 0 or i == numRecords) {

                    std::shared_ptr<hazelcast::client::pipelining<int> > pipe = hazelcast::client::pipelining<int>::create(
                            depth_);

                    pipe->add(hzMap->put_all(**xItr).then(
                            boost::launch::sync, [&](boost::future<void> f) {
                                f.wait();

                                return boost::optional<int>(0);
                            }

                    ));

                    writtenChunks += pipe->results().size();   // <--This is not completing

                    xItr = xTmpMaps.emplace(xTmpMaps.end(), std::make_shared<TMap < K, V>>());

                }

            }

        }

        recordsWritten = std::move(i);

    } catch (hazelcast::client::exception::iexception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    } catch (...) {

        std::cout << "Failed in put(TMap)" << std::endl;

    }

    return recordsWritten;
}


TMap<int, int> populateData() {
    TMap<int, int> our_map;
    for (int i = 0; i < writeChunkSize_ * 1000; ++i) {
        our_map.emplace(i, i);
    }
    return our_map;
}

int main() {
    auto hz = hazelcast::new_client().get();

    clientPool_.emplace_back(HZClient{std::make_shared<hazelcast::client::hazelcast_client>(std::move(hz))});

    TMap<int, int> our_map = populateData();

    put(our_map);

    std::cout << "Finished" << std::endl;

    return 0;
}
