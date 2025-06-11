/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

/**
 * SQL queries are cancellable, so it is possible to fetch
 * first page and ignore the rest by calling `close()`.
 * So it releases all the resources which are occupied by query result.
 */
int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    auto sql = hz.get_sql();

    // infinite stream that will generate 1 sequential long value about every
    // second
    auto result = sql.execute("SELECT * from TABLE(generate_stream(1))").get();

    // Take page iterator
    auto itr = result->iterator();
    auto page_1 = itr.next().get();
    std::cout << "There are " << page_1->row_count()
              << " rows returned from the cluster database in the first page"
              << std::endl;

    // wait 3 seconds to have some data generated
    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto page_2 = itr.next().get();

    std::cout << "There are " << page_2->row_count()
              << " rows returned from the cluster database in the second page. "
              << "Cancelling the sql query execution at the server side now."
              << std::endl;

    // Close the result so that it will cancel the query resources at the server
    // side as well as client side.
    result->close().get();

    std::cout << "Finished" << std::endl;

    return 0;
}
