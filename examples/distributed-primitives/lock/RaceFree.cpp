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
//
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/IAtomicLong.h>
#include <hazelcast/client/ILock.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::IAtomicLong number1 = hz.getIAtomicLong("number1");
    hazelcast::client::IAtomicLong number2 = hz.getIAtomicLong("number2");
    hazelcast::client::ILock lock = hz.getILock("lock");
    std::cout << "Started" << std::endl;
    for (int k = 0; k < 1000000; k++) {
        if (k % 100 == 0) {
            std::cout << "at:" << k << std::endl;
        }

        lock.lock();

        try {
            if (k % 2 == 0) {
                int64_t n1 = number1.get();
                hazelcast::util::sleepmillis(10);
                int64_t n2 = number2.get();
                if (n1 - n2 != 0) {
                    std::cout << "Datarace detected at" << k << " !" << std::endl;
                }
            } else {
                number1.incrementAndGet();
                number2.incrementAndGet();
            }
        } catch (hazelcast::client::exception::IException &e) {
            lock.unlock();
            throw e;
        }

        if (k % 2 == 0) {
            int64_t n1 = number1.get();
            hazelcast::util::sleepmillis(100);
            int64_t n2 = number2.get();
            if (n1 - n2 != 0) {
                std::cout << "Difference detected at" << k << " !" << std::endl;
            }
        } else {
            number1.incrementAndGet();
            number2.incrementAndGet();
        }

        // release the lock
        lock.unlock();
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
