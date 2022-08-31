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
    ASSERT_TRUE(result.row_metadata());
    ASSERT_EQ(2, result.row_metadata()->columns().size());
    auto &column0 = result.row_metadata()->columns()[0];
    EXPECT_EQ("col1", column0.name());
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column0.type());
    EXPECT_TRUE(column0.nullable());
    auto &column1 = result.row_metadata()->columns()[1];
    EXPECT_EQ("col2", column1.name());
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column1.type());
    EXPECT_FALSE(column1.nullable());

    auto page_it = result.page_iterator();
    boost::optional<sql::sql_page> &page = *page_it;
    ASSERT_TRUE(page.has_value());
    auto &rows = page->rows();
    EXPECT_EQ(2, rows.size());
    EXPECT_EQ("foo", rows[0].get_object<std::string>(0).value());
    EXPECT_EQ("bar", rows[0].get_object<std::string>(1).value());
    EXPECT_FALSE(rows[1].get_object<std::string>(0).has_value());
    EXPECT_EQ("hello", rows[1].get_object<std::string>(1).value());
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

    auto page = *result.page_iterator();
    ASSERT_TRUE(page);

    auto &rows = page->rows();
    EXPECT_EQ(1, rows.size());
    EXPECT_EQ("123456", rows[0].get_object<std::string>(0).value());
    EXPECT_EQ("-42.73", rows[0].get_object<std::string>(1).value());
}

TEST_F(SqlTest, exception)
{
    sql::sql_service service = client.get_sql();
    EXPECT_THROW(service.execute("FOO BAR", 42).get(),
                 hazelcast::client::sql::hazelcast_sql_exception);
}

class sql_encode_test : public ::testing::Test {
public:
    sql_encode_test() : random_generator_(std::random_device{}())  {
    }

protected:
    std::mt19937 random_generator_;

    query_id get_query_id() const {
        boost::uuids::uuid server_uuid{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        boost::uuids::uuid client_uuid{21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36};
        return { server_uuid, client_uuid };
    }
};

TEST_F(sql_encode_test, execute)
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

    query_id query_id = get_query_id();

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
        // first frame
        36,  0,   0,   0,   0,   192, 0,
        4,   33,  0,   0,   0,   0,   0,
        0,   0,   0,   0,   255, 255, 255, 255, 42,  0,   0,   0,   0,   0,
        0,   0,   210, 4,   0,   0,   5,   1,
        // second frame (sql string)
        29,  0,   0,   0,   0,   0,
        83,  69,  76,  69,  67,  84,  32,  42,  32,  70,  82,  79,  77,  32,
        115, 111, 109, 101, 119, 104, 101, 114, 101,
        // third frame std::vector<data> encoding for parameters
        6,   0,   0,   0,   0, 16,
        6,   0,   0,   0,   0,  4,
        15,  0,   0,   0,   0,  0,
        115, 111, 109, 101, 98, 121, 116, 101, 115,
        6,   0,   0,   0,   0,  8, // end frame for parameters vector
        15,  0,   0,   0,   0,  0, 109, 121, 45,  115, 99,  104, 101, 109, 97, // schema name string frame
        6,   0,   0,   0,   0, 16, // begin frame for query_id
        38, 0,   0,   0,   0,   0,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,  // server uuid
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, // client uuid
        6,   0,   0,   0,   0,   40 // end frame for query_id
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

TEST_F(sql_encode_test, fetch)
{
    query_id query_id = get_query_id();
    auto msg = hazelcast::client::protocol::codec::sql_fetch_encode(query_id, 7654321);

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    std::vector<unsigned char> expected_bytes = {
        // initial frame
        26, 0, 0,   0,   0,   192, 0,   5,   33,  0,   0,   0,   0,   0,  0, 0,
        0,  0, 255, 255, 255, 255, 177, 203, 116, 0,
        6,   0,   0,   0,   0, 16, // begin frame for query_id
        38, 0,   0,   0,   0,   0,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,  // server uuid
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, // client uuid
        6,   0,   0,   0,   0,   40 // end frame for query_id
    };

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());
    EXPECT_EQ(expected_bytes, actual_bytes);
}

TEST_F(sql_encode_test, close)
{
    query_id query_id = get_query_id();
    auto msg = hazelcast::client::protocol::codec::sql_close_encode(query_id);

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    std::vector<unsigned char> expected_bytes = {
        // initial frame
        22, 0,  0,  0,   0,   192, 0,   3,   33,  0,   0,   0,  0,  0,   0,
        0,  0,  0,  255, 255, 255, 255,
        6,   0,   0,   0,   0, 16, // begin frame for query_id
        38, 0,   0,   0,   0,   0,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,  // server uuid
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, // client uuid
        6,   0,   0,   0,   0,   40 // end frame for query_id
    };

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());
    EXPECT_EQ(expected_bytes, actual_bytes);
}

} // namespace test
} // namespace client
} // namespace hazelcast
