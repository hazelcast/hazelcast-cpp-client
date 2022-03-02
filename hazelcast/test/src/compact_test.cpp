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
#include <atomic>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>
#include <gtest/gtest.h>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include <hazelcast/client/lifecycle_event.h>
#include <hazelcast/client/client_config.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <hazelcast/client/exception/protocol_exceptions.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/hazelcast_json_value.h>
#include <hazelcast/client/imap.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/initial_membership_event.h>
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/internal/socket/SSLSocket.h>
#include <hazelcast/client/lifecycle_listener.h>
#include <hazelcast/client/pipelining.h>
#include <hazelcast/client/serialization_config.h>
#include <hazelcast/client/serialization/pimpl/data_input.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/concurrent/locks/LockSupport.h>
#include <hazelcast/util/MurmurHash3.h>
#include <hazelcast/util/Util.h>

#include "ClientTest.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include "TestHelperFunctions.h"
#include "serialization/Serializables.h"
#include "remote_controller_client.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4996) // for unsafe getenv
#endif

namespace hazelcast {
namespace client {
namespace compact {
namespace test {

class employee
{
public:
    employee() {}

    explicit employee(std::string name)
      : name(std::move(name))
    {}

    const std::string& get_name() const { return name; }

    bool operator==(const employee& rhs) const
    {
        return name == rhs.get_name();
    }

private:
    std::string name;
};

std::ostream&
operator<<(std::ostream& out, const employee& employee)
{
    out << employee.get_name();
    return out;
}

} // namespace test
} // namespace compact

namespace serialization {
template<>
struct hz_serializer<compact::test::employee> : public compact_serializer
{
    static void write(const compact::test::employee& object,
                      compact_writer& writer)
    {
        writer.write_string("name", &object.get_name());
    }

    static compact::test::employee read(compact_reader& reader)
    {
        const boost::optional<std::string>& name = reader.read_string("name");
        return compact::test::employee{ name.get_value_or("default") };
    }

    static std::string type_name() { return "employee"; }
};
} // namespace serialization

namespace compact {
namespace test {
class CompactSerializationTest : public ::testing::Test
{
public:
    template<typename T>
    T to_data_and_back_to_object(serialization::pimpl::SerializationService& ss,
                                 T& value)
    {
        serialization::pimpl::data data = ss.to_data<T>(value);
        return *(ss.to_object<T>(data));
    }
};

TEST_F(CompactSerializationTest, testCustomSerialization)
{
    serialization_config config;
    serialization::pimpl::SerializationService ss(config);

    employee expected("john");
    auto actual = to_data_and_back_to_object(ss, expected);
    ASSERT_EQ(expected, actual);
}

} // namespace test
} // namespace compact
} // namespace client
} // namespace hazelcast