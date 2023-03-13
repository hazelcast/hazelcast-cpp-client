/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/serialization/serialization.h>
#include <iomanip>

/**
 * This example demonstrates how to use generic_record.
 */
int
main()
{
    using namespace hazelcast::client::serialization::generic_record;
    auto client = hazelcast::new_client().get();

    auto map = client.get_map("generic_record_sample_map").get();
    generic_record record = generic_record_builder{ "employee" }
                              .set_int32("age", 21)
                              .set_string("name", "John")
                              .set_int64("id", 11)
                              .build();
    generic_record another_record = generic_record_builder{ "employee" }
                                      .set_int32("age", 19)
                                      .set_string("name", "John")
                                      .set_int64("id", 10)
                                      .build();

    auto cloned_record = record.new_builder_with_clone().build();
    auto updated_cloned_record =
      record.new_builder_with_clone().set_int32("age", 22).build();

    // Put into map and wait
    map->put(1, record).get();
    map->put(2, another_record).get();
    map->put(3, cloned_record).get();
    map->put(4, updated_cloned_record).get();

    std::string variable_names[] = {
        "", "record", "another_record", "cloned_record", "updated_cloned_record"
    };

    for (int i = 1; i < 5; ++i) {
        boost::optional<generic_record> record =
          map->get<int, generic_record>(i).get();

        assert(record.has_value());

        std::cout << std::setw(21) << std::left << variable_names[i] << " => "
                  << "age: " << record->get_int32("age")
                  << ", name:" << record->get_string("name")
                  << ", id:" << record->get_int64("id") << std::endl;
    }

    return 0;
}