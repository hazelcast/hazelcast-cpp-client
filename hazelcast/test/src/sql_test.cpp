#include <gtest/gtest.h>

#include <hazelcast/client/protocol/ClientMessage.h>
#include <hazelcast/client/protocol/codec/codecs.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/sql/sql_statement.h>
#include <hazelcast/client/sql/hazelcast_sql_exception.h>

#include "ClientTest.h"
#include "HazelcastServer.h"
#include "remote_controller_client.h"

using hazelcast::client::protocol::ClientMessage;
using hazelcast::client::serialization::pimpl::data;
using hazelcast::client::sql::impl::query_id;

namespace hazelcast {
namespace client {

namespace test {
struct portable_pojo_key
{
    int64_t key;

    friend bool operator==(const portable_pojo_key& lhs,
                           const portable_pojo_key& rhs);
};
bool
operator==(const portable_pojo_key& lhs, const portable_pojo_key& rhs)
{
    return lhs.key == rhs.key;
}

struct portable_pojo_nested
{
    int32_t val;
};

struct portable_pojo
{
    portable_pojo() = default;

    explicit portable_pojo(int64_t val)
    {
        bool_val = val % 2 == 0;

        tiny_int_val = static_cast<byte>(val);
        small_int_val = static_cast<int16_t>(val);
        int_val = static_cast<int32_t>(val);
        big_int_val = static_cast<int64_t>(val);;
        real_val = static_cast<float>(val);;
        double_val = static_cast<double>(val);;

        char_val = 'c';
        varchar_val = std::to_string(val);

        portable_val = { static_cast<int32_t>(val) };
    }

    portable_pojo(bool bool_val,
                  byte tiny_int_val,
                  int16_t small_int_val,
                  int32_t int_val,
                  int64_t big_int_val,
                  float real_val,
                  double double_val,
                  char char_val,
                  std::string varchar_val,
                  portable_pojo_nested portable_val)
      : bool_val(bool_val)
      , tiny_int_val(tiny_int_val)
      , small_int_val(small_int_val)
      , int_val(int_val)
      , big_int_val(big_int_val)
      , real_val(real_val)
      , double_val(double_val)
      , char_val(char_val)
      , varchar_val(std::move(varchar_val))
      , portable_val(std::move(portable_val))
    {
    }

    bool bool_val;

    byte tiny_int_val;
    int16_t small_int_val;
    int32_t int_val;
    int64_t big_int_val;
    float real_val;
    double double_val;

    char char_val;
    std::string varchar_val;

    portable_pojo_nested portable_val;
};

} // namespace test

namespace serialization {
template<>
class hz_serializer<test::portable_pojo_key> : public portable_serializer
{
public:
    static constexpr int32_t PORTABLE_FACTORY_ID = 1;
    static constexpr int32_t PORTABLE_KEY_CLASS_ID = 2;

    static int32_t get_class_id() noexcept { return PORTABLE_KEY_CLASS_ID; }
    static int32_t get_factory_id() noexcept { return PORTABLE_FACTORY_ID; }
    static void write_portable(const test::portable_pojo_key& object,
                               portable_writer& out)
    {
        out.write("key", object.key);
    }
    static test::portable_pojo_key read_portable(portable_reader& in)
    {
        auto val = in.read<int64_t>("key");
        return { val };
    }
};

constexpr int32_t hz_serializer<test::portable_pojo_key>::PORTABLE_FACTORY_ID;
constexpr int32_t hz_serializer<test::portable_pojo_key>::PORTABLE_KEY_CLASS_ID;

template<>
class hz_serializer<test::portable_pojo_nested> : public portable_serializer
{
public:
    static constexpr int32_t PORTABLE_FACTORY_ID = 1;
    static constexpr int32_t PORTABLE_NESTED_CLASS_ID = 4;

    static int32_t get_class_id() noexcept { return PORTABLE_NESTED_CLASS_ID; }
    static int32_t get_factory_id() noexcept { return PORTABLE_FACTORY_ID; }
    static void write_portable(const test::portable_pojo_nested& object,
                               portable_writer& out)
    {
        out.write("val", object.val);
    }
    static test::portable_pojo_nested read_portable(portable_reader& in)
    {
        auto val = in.read<int32_t>("val");
        return { val };
    }
};

constexpr int32_t
  hz_serializer<test::portable_pojo_nested>::PORTABLE_FACTORY_ID;
constexpr int32_t
  hz_serializer<test::portable_pojo_nested>::PORTABLE_NESTED_CLASS_ID;

template<>
class hz_serializer<test::portable_pojo> : public portable_serializer
{
public:
    static constexpr int32_t PORTABLE_FACTORY_ID = 1;
    static constexpr int32_t PORTABLE_VALUE_CLASS_ID = 3;

    static int32_t get_class_id() noexcept { return PORTABLE_VALUE_CLASS_ID; }
    static int32_t get_factory_id() noexcept { return PORTABLE_FACTORY_ID; }
    static void write_portable(const test::portable_pojo& object,
                               portable_writer& out)
    {
        out.write("booleanVal", object.bool_val);

        out.write("tinyIntVal", object.tiny_int_val);
        out.write("smallIntVal", object.small_int_val);
        out.write("intVal", object.int_val);
        out.write("bigIntVal", object.big_int_val);
        out.write("realVal", object.real_val);
        out.write("doubleVal", object.double_val);

        out.write("charVal", object.char_val);
        out.write("varcharVal", object.varchar_val);

        out.write_portable("portableVal", &object.portable_val);
    }

    static test::portable_pojo read_portable(portable_reader& in)
    {
        return {
            in.read<bool>("booleanVal"),
            in.read<byte>("tinyIntVal"),
            in.read<int16_t>("smallIntVal"),
            in.read<int32_t>("intVal"),
            in.read<int64_t>("bigIntVal"),
            in.read<float>("realVal"),
            in.read<double>("doubleVal"),
            in.read<char>("charVal"),
            in.read<std::string>("varcharVal"),
            in.read_portable<test::portable_pojo_nested>("portableVal").value()
        };
    }
};

constexpr int32_t hz_serializer<test::portable_pojo>::PORTABLE_FACTORY_ID;
constexpr int32_t hz_serializer<test::portable_pojo>::PORTABLE_VALUE_CLASS_ID;

} // namespace serialization
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
namespace std {
template<>
struct DLL_EXPORT hash<hazelcast::client::test::portable_pojo_key>
{
    std::size_t operator()(
      const hazelcast::client::test::portable_pojo_key& k) const noexcept
    {
        return std::hash<int64_t>{}(k.key);
    }
};
} // namespace std

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
    {
    }

protected:
    static void SetUpTestSuite()
    {
        server_factory_.reset(new HazelcastServerFactory(
          "hazelcast/test/resources/hazelcast-sql.xml"));
        member_.reset(new HazelcastServer(*server_factory_));
        member2_.reset(new HazelcastServer(*server_factory_));
    }
    static void TearDownTestSuite()
    {
        member_.reset();
        member2_.reset();
        server_factory_.reset();
    }

    portable_pojo_key key(int64_t i) { return { i }; }

    portable_pojo value(int64_t i) { return portable_pojo{ i }; }

    static const std::vector<std::string>& fields()
    {
        static std::vector<std::string> fields{
            "key",
            "booleanVal",
            "tinyIntVal",
            "smallIntVal",
            "intVal",
            "bigIntVal",
            "realVal",
            "doubleVal",
            //"charVal",
            "varcharVal",
            //"portableVal",
        };

        return fields;
    }

    static const std::vector<sql::sql_column_type> field_types()
    {
        using namespace sql;
        static std::vector<sql_column_type> column_types{
            sql_column_type::bigint,
            sql_column_type::boolean,
            sql_column_type::tinyint,
            sql_column_type::smallint,
            sql_column_type::integer,
            sql_column_type::bigint,
            sql_column_type::real,
            sql_column_type::double_,
            // sql_column_type::varchar,
            sql_column_type::varchar, // sql_column_type::object,
        };

        return column_types;
    }

    static std::string sql(const std::string& map_name)
    {
        auto const& columns = fields();

        std::ostringstream res;
        res << "SELECT ";
        for (std::size_t i = 0; i < columns.size(); ++i) {
            if (i != 0) {
                res << ", ";
            }

            res << columns[i];
        }

        res << " FROM " << map_name;
        return res.str();
    }

    sql::sql_result query(const std::string& map_name)
    {
        auto sql_string = sql(map_name);
        sql::sql_statement s(client, sql_string);
        s.cursor_buffer_size(256);
        return client.get_sql().execute(s).get();
    }

    static void check_row_metada(const sql::sql_row_metadata& row_metadata)
    {
        const auto& columns = fields();
        const auto& column_types = field_types();

        ASSERT_EQ(columns.size(), row_metadata.column_count());

        for (std::size_t i = 0; i < columns.size(); ++i) {
            auto const& field = columns[i];
            auto field_type = column_types[i];

            auto it = row_metadata.find_column(field);
            ASSERT_NE(row_metadata.end(), it);

            auto const& column_meta_data = row_metadata.column(it->second);
            ASSERT_EQ(field, column_meta_data.name);
            ASSERT_EQ(field_type, column_meta_data.type);
            ASSERT_TRUE(column_meta_data.nullable);
        }

        ASSERT_THROW(row_metadata.column(columns.size()),
                     hazelcast::client::exception::index_out_of_bounds);
    }

    template<typename T>
    void check_row_value(sql::sql_column_type expected_type,
                         const T& expected_value,
                         const sql::sql_page::sql_row& row,
                         const std::string& column_name)
    {
        auto column_index = row.row_metadata().find_column(column_name);
        ASSERT_NE(row.row_metadata().end(), column_index);

        ASSERT_EQ(expected_type,
                  row.row_metadata().column(column_index->second).type);

        auto value_by_index = row.template get_object<T>(column_index->second);
        ASSERT_TRUE(value_by_index);
        ASSERT_EQ(expected_value, *value_by_index);

        auto value_by_name = row.template get_object<T>(column_index->first);
        ASSERT_TRUE(value_by_name);
        ASSERT_EQ(expected_value, *value_by_name);
    }

    int member_client_cursors(int member_number) {
        auto script =
          (boost::format("com.hazelcast.jet.sql.SqlTestSupport.sqlInternalService(instance_%1%).getClientStateRegistry().getCursorCount()") %member_number).str();

        Response response;
        remote_controller_client().executeOnController(
          response,
          server_factory_->get_cluster_id(),
          script,
          Lang::PYTHON);
        EXPECT_TRUE(response.success);
        return std::stoi(response.result);

    }

    int total_member_client_cursors() {
        return member_client_cursors(0) + member_client_cursors(1);
    }

private:
    static std::unique_ptr<HazelcastServerFactory> server_factory_;
    static std::unique_ptr<HazelcastServer> member_;
    static std::unique_ptr<HazelcastServer> member2_;
};

std::unique_ptr<HazelcastServerFactory> SqlTest::server_factory_{};
std::unique_ptr<HazelcastServer> SqlTest::member_{};
std::unique_ptr<HazelcastServer> SqlTest::member2_{};

TEST_F(SqlTest, simple)
{
    sql::sql_statement statement(client, R"sql(
        SELECT * FROM (VALUES ('foo', 'bar'), (NULL, 'hello')) AS X(col1, col2)
    )sql");

    sql::sql_service service = client.get_sql();
    auto result = service.execute(statement).get();

    ASSERT_TRUE(result.row_set());
    EXPECT_EQ(-1, result.update_count());
    ASSERT_NO_THROW(result.row_metadata());
    ASSERT_EQ(2, result.row_metadata().columns().size());
    auto& column0 = result.row_metadata().columns()[0];
    EXPECT_EQ("col1", column0.name);
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column0.type);
    EXPECT_TRUE(column0.nullable);
    auto& column1 = result.row_metadata().columns()[1];
    EXPECT_EQ("col2", column1.name);
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column1.type);
    EXPECT_FALSE(column1.nullable);

    auto page_it = result.page_iterator();
    auto const& page = *page_it;
    ASSERT_TRUE(page.has_value());
    auto& rows = page->rows();
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

    ASSERT_TRUE(result.row_set());
    EXPECT_EQ(-1, result.update_count());

    auto page = *result.page_iterator();
    ASSERT_TRUE(page);

    auto& rows = page->rows();
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

// ported from Java SqlBasicClientTest.testSelect
TEST_F(SqlTest, select)
{
    sql::sql_service service = client.get_sql();

    auto map_name = get_test_name();

    auto map = client.get_map(map_name).get();

    constexpr std::size_t DATA_SET_SIZE = 4096;
    std::unordered_map<portable_pojo_key, portable_pojo> entries(DATA_SET_SIZE);
    for (std::size_t i = 0; i < DATA_SET_SIZE; ++i) {
        entries[key(i)] = value(i);
    }

    map->put_all(entries).get();

    ASSERT_EQ(DATA_SET_SIZE, map->size().get());

    auto sql =
      (boost::format("CREATE OR REPLACE MAPPING %1% ("
                     "key BIGINT EXTERNAL NAME \"__key.key\", "
                     "booleanVal BOOLEAN, "
                     "tinyIntVal TINYINT, "
                     "smallIntVal SMALLINT, "
                     "intVal INTEGER, "
                     "bigIntVal BIGINT, "
                     "realVal REAL, "
                     "doubleVal DOUBLE, "
                     //"charVal VARCHAR, "
                     "varcharVal VARCHAR "
                     ") TYPE IMap OPTIONS( "
                     "'keyFormat'='portable'"
                     ", 'keyPortableFactoryId'='%2%'"
                     ", 'keyPortableClassId'='%3%'"
                     ", 'keyPortableClassVersion'='0'"
                     ", 'valueFormat'='portable'"
                     ", 'valuePortableFactoryId'='%4%'"
                     ", 'valuePortableClassId'='%5%'"
                     ", 'valuePortableClassVersion'='0'"
                     ")") %
       map_name %
       serialization::hz_serializer<portable_pojo_key>::PORTABLE_FACTORY_ID %
       serialization::hz_serializer<portable_pojo_key>::PORTABLE_KEY_CLASS_ID %
       serialization::hz_serializer<portable_pojo>::PORTABLE_FACTORY_ID %
       serialization::hz_serializer<portable_pojo>::PORTABLE_VALUE_CLASS_ID)
        .str();

    using namespace sql;

    sql_result result;
    ASSERT_NO_THROW(result = service.execute(sql).get());

    sql_result res = query(map_name);

    auto const& row_metadata = res.row_metadata();
    check_row_metada(row_metadata);

    std::unordered_set<int64_t> unique_keys;
    auto it = res.page_iterator();
    for (;it; (++it).get()) {
        auto const& page = *it;
        for (const auto& row : page->rows()) {
            ASSERT_EQ(row_metadata, res.row_metadata());

            auto key0 =
              row.get_object<int64_t>(row_metadata.find_column("key")->second);
            ASSERT_TRUE(key0);

            portable_pojo_key key{ *key0 };
            auto value0 = map->get<portable_pojo_key, portable_pojo>(key).get();
            ASSERT_TRUE(value0);
            auto const& val = *value0;

            check_row_value(sql_column_type::bigint, key.key, row, "key");
            check_row_value(
              sql_column_type::boolean, val.bool_val, row, "booleanVal");
            check_row_value(
              sql_column_type::tinyint, val.tiny_int_val, row, "tinyIntVal");
            check_row_value(
              sql_column_type::smallint, val.small_int_val, row, "smallIntVal");
            check_row_value(
              sql_column_type::integer, val.int_val, row, "intVal");
            check_row_value(
              sql_column_type::bigint, val.big_int_val, row, "bigIntVal");
            check_row_value(
              sql_column_type::real, val.real_val, row, "realVal");
            check_row_value(
              sql_column_type::double_, val.double_val, row, "doubleVal");
            check_row_value(
              sql_column_type::varchar, val.varchar_val, row, "varcharVal");

            unique_keys.emplace(*key0);

            ASSERT_THROW(row.get_object<int>(-1),
                         exception::index_out_of_bounds);
            ASSERT_THROW(row.get_object<int>(row.row_metadata().column_count()),
                         exception::index_out_of_bounds);
            ASSERT_THROW(row.get_object<int>("unknown_field"), exception::illegal_argument);
        }
    }

    ASSERT_THROW(res.page_iterator(), exception::illegal_state);

    ASSERT_EQ(DATA_SET_SIZE, unique_keys.size());

    ASSERT_NO_THROW(res.close().get());

    // If this request spawns multiple pages, then:
    // 1) Ensure that results are cleared when the whole result set is fetched
    // 2) Ensure that results are cleared when the result set is closed in the middle.
/*
    ASSERT_EQ(0, total_member_client_cursors());

    try {
        auto res2 = query(map_name);

        ASSERT_EQ(1, total_member_client_cursors());

        ASSERT_NO_THROW(res2.close());

        ASSERT_EQ(0, total_member_client_cursors());
    } catch (...) {
        
    }
*/
}

class sql_encode_test : public ::testing::Test
{
public:
    sql_encode_test() {}

protected:
    query_id get_query_id() const
    {
        boost::uuids::uuid server_uuid{ 1, 2,  3,  4,  5,  6,  7,  8,
                                        9, 10, 11, 12, 13, 14, 15, 16 };
        boost::uuids::uuid client_uuid{ 21, 22, 23, 24, 25, 26, 27, 28,
                                        29, 30, 31, 32, 33, 34, 35, 36 };
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
    auto msg =
      hazelcast::client::protocol::codec::sql_fetch_encode(query_id, 7654321);

    std::vector<unsigned char> actual_bytes = {};
    for (auto buf : msg.get_buffer()) {
        actual_bytes.insert(actual_bytes.end(), buf.begin(), buf.end());
    }

    std::vector<unsigned char> expected_bytes = {
        // initial frame
        26,  0,   0,   0,  0,  192, 0,  5,   33,  0,   0,
        0,   0,   0,   0,  0,  0,   0,  255, 255, 255, 255,
        177, 203, 116, 0,  6,  0,   0,  0,   0,   16, // begin frame for
                                                      // query_id
        38,  0,   0,   0,  0,  0,   1,  2,   3,   4,   5,
        6,   7,   8,   9,  10, 11,  12, 13,  14,  15,  16, // server uuid
        21,  22,  23,  24, 25, 26,  27, 28,  29,  30,  31,
        32,  33,  34,  35, 36,    // client uuid
        6,   0,   0,   0,  0,  40 // end frame for query_id
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

    std::vector<unsigned char>
      expected_bytes = {
          // initial frame
          22, 0,  0,  0,   0,   192, 0,   3,  33, 0,  0,  0,  0,  0,  0,
          0,  0,  0,  255, 255, 255, 255, 6,  0,  0,  0,  0,  16, // begin frame
                                                                  // for
                                                                  // query_id
          38, 0,  0,  0,   0,   0,   1,   2,  3,  4,  5,  6,  7,  8,  9,
          10, 11, 12, 13,  14,  15,  16, // server uuid
          21, 22, 23, 24,  25,  26,  27,  28, 29, 30, 31, 32, 33, 34, 35,
          36,                      // client uuid
          6,  0,  0,  0,   0,   40 // end frame for query_id
      };

    ASSERT_EQ(actual_bytes.size(), expected_bytes.size());
    EXPECT_EQ(expected_bytes, actual_bytes);
}

} // namespace test
} // namespace client
} // namespace hazelcast
