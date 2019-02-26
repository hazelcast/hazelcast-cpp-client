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
#include <hazelcast/client/adaptor/RawPointerTransactionalMap.h>

int main() {
    hazelcast::client::ClientConfig config;
    hazelcast::client::HazelcastClient hz(config);

    hazelcast::client::TransactionContext txCtxt = hz.newTransactionContext();

    txCtxt.beginTransaction();

    hazelcast::client::TransactionalMap<std::string, std::string> transactionalMap = txCtxt.getMap<std::string, std::string>("txMap");

    hazelcast::client::adaptor::RawPointerTransactionalMap<std::string, std::string> map(transactionalMap);
    map.put("1", "Tokyo");
    map.put("2", "Paris");
    map.put("3", "New York");

    txCtxt.commitTransaction();

    std::cout << "Finished loading transactional map" << std::endl;

    return 0;
}
