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

    auto q1 = hz.get_queue("q");
    auto q2 = hz.get_queue("q");

    q1->put("foo").get();
    std::cout << "q1->size:" << q1->size().get() << "  q2->size:" << q2->size().get() << std::endl;

    q1->destroy().get();
    std::cout << "q1->size:" << q1->size().get() << "  q2->size:" << q2->size().get() << std::endl;

    std::cout << "Finished" << std::endl;

    return 0;
}
