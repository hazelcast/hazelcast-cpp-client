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
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/ISemaphore.h>
#include <hazelcast/client/IAtomicLong.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::ISemaphore semaphore = hz.getISemaphore("semaphore");
    hazelcast::client::IAtomicLong resource = hz.getIAtomicLong("resource");
    for (int k = 0; k < 1000; k++) {
        std::cout << "At iteration: " << k << ", Active Threads: " << resource.get() << std::endl;
        semaphore.acquire();
        try {
            resource.incrementAndGet();
            hazelcast::util::sleep(1);
            resource.decrementAndGet();
        } catch (hazelcast::client::exception::IException &e) {
            semaphore.release();
            throw e;
        }
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
