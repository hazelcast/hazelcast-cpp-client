#include <gtest/gtest.h>

#include <hazelcast/client/protocol/ClientMessage.h>
#include <hazelcast/client/protocol/codec/codecs.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/sql/sql_statement.h>
#include <hazelcast/client/sql/hazelcast_sql_exception.h>

#include "ClientTest.h"
#include "HazelcastServer.h"

using hazelcast::client::protocol::ClientMessage;
using hazelcast::client::serialization::pimpl::data;
using hazelcast::client::sql::impl::query_id;

namespace hazelcast {
namespace client {
namespace test {

class SqlTest : public ClientTest
{
public:
    hazelcast_client client;

    static client_config get_config()
    {
        client_config cfg = ClientTest::get_config();
        cfg.set_cluster_name("sql-dev");
        return cfg;
    }

    SqlTest()
      : client{ hazelcast::new_client(get_config()).get() }
    {}

protected:
    static void SetUpTestSuite()
    {
        server_factory_.reset(new HazelcastServerFactory(
          "hazelcast/test/resources/hazelcast-sql.xml"));
        member_.reset(new HazelcastServer(*server_factory_));
    }
    static void TearDownTestSuite()
    {
        member_.reset();
        server_factory_.reset();
    }

private:
    static std::unique_ptr<HazelcastServerFactory> server_factory_;
    static std::unique_ptr<HazelcastServer> member_;
};

std::unique_ptr<HazelcastServerFactory> SqlTest::server_factory_{};
std::unique_ptr<HazelcastServer> SqlTest::member_{};

TEST_F(SqlTest, simple)
{
    sql::sql_statement statement(client, R"sql(
        SELECT * FROM (VALUES ('foo', 'bar'), (NULL, 'hello')) AS X(col1, col2)
    )sql");

    sql::sql_service service = client.get_sql();
    auto result = service.execute(statement).get();

    ASSERT_TRUE(result.is_row_set());
    EXPECT_EQ(-1, result.update_count());
    ASSERT_TRUE(result.row_metadata().has_value());
    ASSERT_EQ(2, result.row_metadata()->size());
    EXPECT_EQ("col1", result.row_metadata().value()[0].name());
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar,
              result.row_metadata().value()[0].type());
    EXPECT_TRUE(result.row_metadata().value()[0].nullable());
    EXPECT_EQ("col2", result.row_metadata().value()[1].name());
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar,
              result.row_metadata().value()[1].type());
    EXPECT_FALSE(result.row_metadata().value()[1].nullable());

    auto rows = result.fetch_page().get();

    EXPECT_EQ(2, rows.size());
    EXPECT_EQ("foo", rows[0].get_value<std::string>(0).value());
    EXPECT_EQ("bar", rows[0].get_value<std::string>(1).value());
    EXPECT_FALSE(rows[1].get_value<std::string>(0).has_value());
    EXPECT_EQ("hello", rows[1].get_value<std::string>(1).value());
}

TEST_F(SqlTest, statement_with_params)
{
    sql::sql_service service = client.get_sql();
    auto result = service
                    .execute("SELECT CAST(? AS VARCHAR), CAST(? AS VARCHAR)",
                             123456,
                             -42.73)
                    .get();

    ASSERT_TRUE(result.is_row_set());
    EXPECT_EQ(-1, result.update_count());

    auto rows = result.fetch_page().get();

    EXPECT_EQ(1, rows.size());
    EXPECT_EQ("123456", rows[0].get_value<std::string>(0).value());
    EXPECT_EQ("-42.73", rows[0].get_value<std::string>(1).value());
}

TEST_F(SqlTest, exception)
{
    sql::sql_service service = client.get_sql();
    EXPECT_THROW(service.execute("FOO BAR", 42).get(),
                 hazelcast::client::sql::hazelcast_sql_exception);
}

} // namespace test
} // namespace client
} // namespace hazelcast

TEST(sql_encode_test, execute)
{
    std::string const query = "SELECT * FROM somewhere";

    std::vector<data> params;
    params.emplace_back(data());
    params.emplace_back(
      data{ { 's', 'o', 'm', 'e', 'b', 'y', 't', 'e', 's' } });

    int timeout_millis = 42;
    int curser_buffer_size = 1234;

    std::string const schema = "my-schema";

    unsigned char expected_res_type = 5;

    query_id query_id{ 7777777LL, 88888888LL, 999999999LL, -1LL };

    bool skip_update_statistics = true;

    auto msg = hazelcast::client::protocol::codec::sql_execute_encode(
      query,
      params,
      timeout_millis,
      curser_buffer_size,
      &schema,
      expected_res_type,
      query_id,
      skip_update_statistics);

    std::vector<unsigned char> expected_bytes = {
        36,  0,   0,   0,   0,   192, 0,   4,   33,  0,   0,   0,   0,   0,
        0,   0,   0,   0,   255, 255, 255, 255, 42,  0,   0,   0,   0,   0,
        0,   0,   210, 4,   0,   0,   5,   1,   29,  0,   0,   0,   0,   0,
        83,  69,  76,  69,  67,  84,  32,  42,  32,  70,  82,  79,  77,  32,
        115, 111, 109, 101, 119, 104, 101, 114, 101, 6,   0,   0,   0,   0,
        16,  6,   0,   0,   0,   0,   4,   15,  0,   0,   0,   0,   0,   115,
        111, 109, 101, 98,  121, 116, 101, 115, 6,   0,   0,   0,   0,   8,
        15,  0,   0,   0,   0,   0,   109, 121, 45,  115, 99,  104, 101, 109,
        97,  6,   0,   0,   0,   0,   16,  38,  0,   0,   0,   0,   0,   241,
        173, 118, 0,   0,   0,   0,   0,   56,  86,  76,  5,   0,   0,   0,
        0,   255, 201, 154, 59,  0,   0,   0,   0,   255, 255, 255, 255, 255,
        255, 255, 255, 6,   0,   0,   0,   0,   40
    };

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    for (auto b : actual_bytes) {
        std::cout << unsigned(b) << ", ";
    }
    std::cout << std::endl;

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());

    EXPECT_EQ(expected_bytes, actual_bytes);
}

TEST(sql_encode_test, fetch)
{
    auto msg = hazelcast::client::protocol::codec::sql_fetch_encode(
      query_id{ 1000000000000000000LL, 0, 1, 2 }, 7654321);

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    std::vector<unsigned char> expected_bytes = {
        26, 0, 0,   0,   0,   192, 0,   5,   33,  0,   0,   0,   0,   0,  0, 0,
        0,  0, 255, 255, 255, 255, 177, 203, 116, 0,   6,   0,   0,   0,  0, 16,
        38, 0, 0,   0,   0,   0,   0,   0,   100, 167, 179, 182, 224, 13, 0, 0,
        0,  0, 0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,  2, 0,
        0,  0, 0,   0,   0,   0,   6,   0,   0,   0,   0,   40
    };

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());
    EXPECT_EQ(expected_bytes, actual_bytes);
}

TEST(sql_encode_test, close)
{
    auto msg = hazelcast::client::protocol::codec::sql_close_encode(query_id{
      1000000000000000000LL, -1000000000000000000LL, 0, 111111111111111LL });

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    std::vector<unsigned char> expected_bytes = {
        22, 0,  0,  0,   0,   192, 0,   3,   33,  0,   0,   0,  0,  0,   0,
        0,  0,  0,  255, 255, 255, 255, 6,   0,   0,   0,   0,  16, 38,  0,
        0,  0,  0,  0,   0,   0,   100, 167, 179, 182, 224, 13, 0,  0,   156,
        88, 76, 73, 31,  242, 0,   0,   0,   0,   0,   0,   0,  0,  199, 241,
        78, 18, 14, 101, 0,   0,   6,   0,   0,   0,   0,   40
    };

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());
    EXPECT_EQ(expected_bytes, actual_bytes);
}
