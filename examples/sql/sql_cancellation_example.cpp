/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    // populate the map with some data
    auto map = hz.get_map("integer").get();
    for (int i = 0; i < 100; ++i) {
        map->put(i, i).get();
    }

    auto sql = hz.get_sql();

    // infinite stream that will generate 1 sequential long value about every
    // second
    auto result = sql.execute("SELECT * from TABLE(generate_stream(1))").get();

    auto it = result->page_iterator();
    std::cout << "There are " << (*it)->row_count()
              << " rows returned from the cluster database in the first page"
              << std::endl;

    // wait 3 seconds to have some data generated
    std::this_thread::sleep_for(std::chrono::seconds(3));

    (++it).get(); // this will block until the next page is ready

    std::cout << "There are " << (*it)->row_count()
              << " rows returned from the cluster database in the 2nd page. "
              << "Cancelling the sql query execution at the server side now."
              << std::endl;

    // Close the result so that it will cancel the query resources at the server
    // side as well as client side.
    result->close().get();

    std::cout << "Finished" << std::endl;

    return 0;
}
