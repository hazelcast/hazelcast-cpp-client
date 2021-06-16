/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/hazelcast.h>
#include <hazelcast/client/query/predicates.h>

using namespace hazelcast::client;

struct User
{
    friend std::ostream& operator<<(std::ostream& os, const User& user)
    {
        os << "User{"
           << " username: " << user.username << " age: " << user.age << " active: " << user.active
           << '}';
        return os;
    }

    std::string username;
    int32_t age;
    bool active;
};

namespace hazelcast {
namespace client {
namespace serialization {
template<>
struct hz_serializer<User> : portable_serializer
{
    static int32_t get_factory_id() noexcept { return 1; }

    static int32_t get_class_id() noexcept { return 3; }

    static void write_portable(const User& object,
                               hazelcast::client::serialization::portable_writer& out)
    {
        out.write("username", object.username);
        out.write("age", object.age);
        out.write("active", object.active);
    }

    static User read_portable(hazelcast::client::serialization::portable_reader& in)
    {
        return User{ in.read<std::string>("username"),
                     in.read<int32_t>("age"),
                     in.read<bool>("active") };
    }
};
} // namespace serialization
} // namespace client
} // namespace hazelcast

void
generate_users(std::shared_ptr<imap> users)
{
    users->put<std::string, User>("Rod", User{ "Rod", 19, true }).get();
    users->put<std::string, User>("Jane", User{ "Jane", 20, true }).get();
    users->put<std::string, User>("Freddy", User{ "Freddy", 23, true }).get();
}

int
main()
{
    auto hz = hazelcast::new_client().get();
    // Get a Distributed Map called "users"
    auto users = hz.get_map("users").get();
    // Add some users to the Distributed Map
    generate_users(users);
    // Create a Predicate from a String (a SQL like Where clause)
    // Creating the same Predicate as above but with and_predicate builder
    query::and_predicate criteriaQuery(
      hz, query::equal_predicate(hz, "active", true), query::between_predicate(hz, "age", 18, 21));
    // Get result collections using the two different Predicates
    // Use SQL Query
    auto result1 =
      users->values<User>(query::sql_predicate(hz, "active AND age BETWEEN 18 AND 21)")).get();
    auto result2 = users->values<User>(criteriaQuery).get();
    // Print out the results
    std::cout << "Result 1:" << std::endl;
    for (auto& value : result1) {
        std::cout << value << std::endl;
    }
    std::cout << "Result 2:" << std::endl;
    for (auto& value : result2) {
        std::cout << value << std::endl;
    }
    hz.shutdown().get();

    return 0;
}
