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

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::TransactionOptions txOptions;
    txOptions.setTimeout(10);

    hazelcast::client::TransactionContext txCxt = hz.newTransactionContext(txOptions);

    try {
        txCxt.beginTransaction();

        hazelcast::client::TransactionalMap<std::string, std::string> map = txCxt.getMap<std::string, std::string>(
                "transaction map");

        map.put("1", "1");
        hazelcast::util::sleepmillis(20);
        map.put("2", "2");
        txCxt.commitTransaction();
    } catch (hazelcast::client::exception::IException &e) {
        txCxt.rollbackTransaction();
        std::cerr << "Transaction failed !!! " << e.what() << std::endl;
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}

