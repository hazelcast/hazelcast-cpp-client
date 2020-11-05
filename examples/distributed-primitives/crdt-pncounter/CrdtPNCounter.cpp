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

    auto pnCounter = hz.get_pn_counter("pncounterexample");

    std::cout << "Counter started with value:" << pnCounter->get().get() << std::endl;

    std::cout << "Counter new value after adding is: " << pnCounter->add_and_get(5).get() << std::endl;

    std::cout << "Counter new value before adding is: " << pnCounter->get_and_add(2).get() << std::endl;

    std::cout << "Counter new value is: " << pnCounter->get().get() << std::endl;

    std::cout << "Decremented counter by one to: " << pnCounter->decrement_and_get().get() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
