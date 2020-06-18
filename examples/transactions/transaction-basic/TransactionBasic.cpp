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
#include <hazelcast/client/HazelcastClient.h>

int main() {
    hazelcast::client::HazelcastClient hz;

    hazelcast::client::TransactionOptions txOptions;
    txOptions.setTimeout(std::chrono::seconds(10));

    hazelcast::client::TransactionContext txCxt = hz.newTransactionContext(txOptions);

    try {
        txCxt.beginTransaction().get();

        auto map = txCxt.getMap("transaction map");

        map->put<std::string, std::string>("1", "1").get();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        map->put<std::string, std::string>("2", "2").get();
        txCxt.commitTransaction().get();
    } catch (hazelcast::client::exception::IException &e) {
        txCxt.rollbackTransaction().get();
        std::cerr << "Transaction failed !!! " << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

