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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/ICountDownLatch.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::ICountDownLatch latch = hz.getICountDownLatch("countDownLatch");

    std::cout << "Starting" << std::endl;

    //we init the latch with 1, since we only need to complete a single step.
    latch.trySetCount(1);

    //do some sleeping to simulate doing something
    hazelcast::util::sleep(30);

    //now we do a countdown which notifies all followers
    latch.countDown();

    std::cout << "Leader finished" << std::endl;

    //we need to clean up the latch
    latch.destroy();

    std::cout << "Finished" << std::endl;

    return 0;
}
