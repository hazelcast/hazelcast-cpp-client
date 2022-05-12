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

/**
 * This example shows how to use mixed type collections. It can also be used to
 * prevent de-serialization and do late de-serialization. You can query the type
 * information of a value during runtime and take action based on the type.
 */

#include <hazelcast/client/hazelcast_client.h>
#include <ostream>

int
main()
{
    using namespace hazelcast::client;

    auto hz = hazelcast::new_client().get();

    auto map = hz.get_map("map").get();

    boost::optional<typed_data> the_value =
      map->get<std::string, typed_data>("key").get();
    if (the_value) {
        std::cout << "value type id is: " << the_value->get_type() << std::endl;
        if (the_value->get_type().type_id ==
            serialization::pimpl::serialization_constants::
              CONSTANT_TYPE_BOOLEAN) {
            // we know for sure that the value is of type `bool` and we can get
            // it with no exception
            bool value = *the_value->get<bool>();
            std::cout << "value is: " << value << std::endl;
        }
    } else {
        std::cout << "value is not set" << std::endl;
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
