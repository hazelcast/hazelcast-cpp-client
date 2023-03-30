#include <gtest/gtest.h>

#include <limits>
#include <future>
#include <boost/algorithm/string.hpp>

#include <hazelcast/client/protocol/ClientMessage.h>
#include <hazelcast/client/protocol/codec/codecs.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/sql/sql_statement.h>
#include <hazelcast/client/sql/hazelcast_sql_exception.h>
#include <hazelcast/client/sql/impl/sql_error_code.h>
#include <hazelcast/client/sql/impl/read_optimized_lru_cache.h>

#include "ClientTest.h"
#include "HazelcastServer.h"
#include "TestHelperFunctions.h"
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
        big_int_val = static_cast<int64_t>(val);
        real_val = static_cast<float>(val);
        double_val = static_cast<double>(val);

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

struct student
{
    int64_t age;
    float height;

    friend inline bool operator==(const student& x, const student& y)
    {
        return x.age == y.age && x.height == y.height;
    }
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

template<>
struct hz_serializer<test::student> : public portable_serializer
{
    static constexpr int32_t PORTABLE_FACTORY_ID = 1;
    static constexpr int32_t PORTABLE_VALUE_CLASS_ID = 5;

    static int32_t get_class_id() noexcept { return PORTABLE_VALUE_CLASS_ID; }
    static int32_t get_factory_id() noexcept { return PORTABLE_FACTORY_ID; }

    static void write_portable(const test::student& x, portable_writer& out)
    {
        out.write("age", x.age);
        out.write("height", x.height);
    }

    static test::student read_portable(portable_reader& in)
    {
        return { in.read<int64_t>("age"), in.read<float>("height") };
    }
};

constexpr int32_t hz_serializer<test::student>::PORTABLE_FACTORY_ID;
constexpr int32_t hz_serializer<test::student>::PORTABLE_VALUE_CLASS_ID;

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

enum class iterator_type
{
    page_iterator,
    page_iterator_sync,
    row_iterator_sync
};

class SqlTest
  : public ClientTest
  , public ::testing::WithParamInterface<iterator_type>
{
public:
    static std::unique_ptr<HazelcastServerFactory> server_factory_;
    static std::unique_ptr<HazelcastServer> member_;
    static std::unique_ptr<HazelcastServer> member2_;
    static std::unique_ptr<HazelcastServer> member3_;
    hazelcast_client client;
    std::string map_name;
    imap_t map;

    SqlTest()
      : client{ hazelcast::new_client(get_config()).get() }
      , map_name{ random_map_name() }
      , map{ client.get_map(map_name).get() }
    {
    }

    ~SqlTest() { client.shutdown().get(); }

    static client_config get_config()
    {
        client_config cfg = ClientTest::get_config();
        cfg.set_cluster_name("sql-dev");
        return cfg;
    }

protected:
    static void SetUpTestSuite()
    {
        std::string config_file_path =
          "hazelcast/test/resources/hazelcast-sql.xml";

        // Jet doesn't exist before v5.0.0
        if (cluster_version() < member::version{ 5, 0, 0 })
            config_file_path =
              "hazelcast/test/resources/hazelcast-sql-without-jet.xml";

        server_factory_.reset(new HazelcastServerFactory(config_file_path));
        member_.reset(new HazelcastServer(*server_factory_));
        member2_.reset(new HazelcastServer(*server_factory_));
        member3_.reset(new HazelcastServer(*server_factory_));
    }

    void TearDown() override
    {
        try {
            map->destroy().get();
        } catch (...) {
        }
    }

    static void TearDownTestSuite()
    {
        member_.reset();
        member2_.reset();
        member3_.reset();
        server_factory_.reset();
    }

    void SetUp() override
    {
        // SQL API messages are not supported before 4.2.0
        if (cluster_version() < member::version{ 4, 2, 0 }) {
            GTEST_SKIP();
        }
    }


    void create_mapping(std::string value_format = "INTEGER",
                        boost::optional<std::string> par_map_name = boost::none)
    {
        // Mapping is not supported before 5.0.0
        if (cluster_version() < member::version{ 5, 0, 0 })
            return;

        std::string curr_map_name = map_name;
        if (par_map_name.has_value())
            curr_map_name = *par_map_name;
        std::string query =
          (boost::format("CREATE MAPPING %1% ( "
                         "__key INT, "
                         "this %2% "
                         ") "
                         "TYPE IMap "
                         "OPTIONS ("
                         "'keyFormat' = 'int', "
                         "'valueFormat' = '%3%' "
                         ")") %
           curr_map_name % value_format % boost::to_lower_copy(value_format))
            .str();

        client.get_sql().execute(query).get();
    }

    void create_mapping_for_portable(std::string mapping_query,
                                     int factory_id,
                                     int class_id)
    {
        if (cluster_version() < member::version{ 5, 0, 0 })
            return;

        std::string query = (boost::format("CREATE MAPPING %1% ( "
                                           "__key INT, "
                                           "%2%"
                                           ") "
                                           "TYPE IMap "
                                           "OPTIONS ("
                                           "'keyFormat' = 'int', "
                                           "'valueFormat' = 'portable', "
                                           "'valuePortableFactoryId' = '%3%', "
                                           "'valuePortableClassId' = '%4%'"
                                           ")") %
                             map_name % mapping_query % factory_id % class_id)
                              .str();

        client.get_sql().execute(query).get();
    }

    void create_mapping_for_portable_as_key(int factory_id, int class_id)
    {
        if (cluster_version() < member::version{ 5, 0, 0 })
            return;

        std::string query = (boost::format("CREATE MAPPING %1% "
                                           "TYPE IMap "
                                           "OPTIONS ("
                                           "'keyFormat' = 'portable', "
                                           "'valueFormat' = 'varchar', "
                                           "'keyPortableFactoryId' = '%2%', "
                                           "'keyPortableClassId' = '%3%'"
                                           ")") %
                             map_name % factory_id % class_id)
                              .str();

        client.get_sql().execute(query).get();
    }

    void create_mapping_for_student()
    {
        create_mapping_for_portable(
          R"(
                age BIGINT,
                height REAL
            )",
          serialization::hz_serializer<test::student>::PORTABLE_FACTORY_ID,
          serialization::hz_serializer<test::student>::PORTABLE_VALUE_CLASS_ID);
    }

    void create_mapping_for_student_as_key()
    {
        create_mapping_for_portable_as_key(
          serialization::hz_serializer<test::student>::PORTABLE_FACTORY_ID,
          serialization::hz_serializer<test::student>::PORTABLE_VALUE_CLASS_ID);
    }
    
    std::shared_ptr<sql::sql_result> select_all(int cursor_size = 10)
    {
        sql::sql_statement statement{ client,
                        (boost::format(
                                R"(
                SELECT * FROM %1%
            )") % map_name).str() };

        statement.cursor_buffer_size(cursor_size);
        return client.get_sql().execute(statement).get();
    }

    portable_pojo_key key(int64_t i) { return { i }; }

    portable_pojo value(int64_t i) { return portable_pojo{ i }; }

    int32_t get_direct_imap_queries_executed(int32_t instance_index)
    {
        auto script_template =
          (boost::format(
             "var optimizer = "
             "Java.type('com.hazelcast.jet.sql.impl.CalciteSqlOptimizer'); \n"
             "optimizer = "
             "instance_%1%.getOriginal().node.nodeEngine.getSqlService()."
             "getOptimizer();\n"
             "result = \"\" + "
             "optimizer.getPlanExecutor().getDirectIMapQueriesExecuted();\n") %
           instance_index)
            .str();

        Response response;
        remote_controller_client().executeOnController(
          response,
          server_factory_->get_cluster_id(),
          std::string(script_template),
          Lang::JAVASCRIPT);

        EXPECT_TRUE(response.success);
        return std::stoi(response.result);
    }

    std::string get_uuid_of_instance(int32_t instance_index)
    {
        auto script_template =
          (boost::format("result = \"\" + "
                         "instance_%1%.getOriginal().node.nodeEngine."
                         "getLocalMember().getUuid();\n") %
           instance_index)
            .str();

        Response response;
        remote_controller_client().executeOnController(
          response,
          server_factory_->get_cluster_id(),
          std::string(script_template),
          Lang::JAVASCRIPT);

        EXPECT_TRUE(response.success);
        return response.result;
    }

    std::vector<int32_t> prepare_member_to_instance_mapping()
    {
        std::vector<member> members = client.get_cluster().get_members();
        std::vector<int32_t> instance_mapping(members.size());

        for (size_t i = 0; i < members.size(); i++) {
            std::string curr_uuid = get_uuid_of_instance(i);

            for (size_t j = 0; j < members.size(); j++) {
                if (curr_uuid == to_string(members[j].get_uuid())) {
                    instance_mapping[j] = i;
                }
            }
        }
        return instance_mapping;
    }

    struct assert_row_count
    {
        using sql_row = sql::sql_page::sql_row;

        std::size_t expected;
        std::size_t actual{};

        assert_row_count(std::size_t expect)
          : expected{ expect }
        {
        }

        ~assert_row_count() { EXPECT_EQ(actual, expected); }

        void operator()(const sql_row& row) { ++actual; }
    };

    template<typename T>
    struct assert_entries_equal
    {
        using sql_row = sql::sql_page::sql_row;

        std::unordered_map<int, T> expecteds;

        void operator()(const sql_row& row) const
        {
            auto idx = row.get_object<int>(0);

            EXPECT_TRUE(idx.has_value());
            auto value = row.get_object<T>(1);
            EXPECT_TRUE(value.has_value());
            EXPECT_EQ(expecteds.at(*idx), *value);
        }
    };

    template<typename... Fns>
    void for_each_row(std::shared_ptr<sql::sql_result> result, Fns&&... fn)
    {
        for_each_row_until(std::numeric_limits<int64_t>::max(),
                           move(result),
                           std::forward<Fns>(fn)...);
    }

    template<typename... Fns>
    void for_each_row_until(int64_t n_rows,
                            std::shared_ptr<sql::sql_result> result,
                            Fns&&... fn)
    {
        switch (GetParam()) {
            case iterator_type::page_iterator: {
                for (auto itr = result->iterator(); itr.has_next();) {
                    auto page = itr.next().get();

                    for (auto const& row : page->rows()) {
                        int _[] = { 0, ((void)fn(row), 0)... };
                        (void)_;

                        if (!--n_rows)
                            return;
                    }
                }
            } break;
            case iterator_type::page_iterator_sync: {
                for (auto itr = result->pbegin(); itr != result->pend();
                     ++itr) {
                    auto page = *itr;

                    for (auto const& row : page->rows()) {
                        int _[] = { 0, ((void)fn(row), 0)... };
                        (void)_;

                        if (!--n_rows)
                            return;
                    }
                }
            } break;
            case iterator_type::row_iterator_sync: {
                for (const auto& row : *result) {
                    int _[] = { 0, ((void)fn(row), 0)... };
                    (void)_;

                    if (!--n_rows)
                        return;
                }
            }
        }
    }

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

    std::shared_ptr<sql::sql_result> query(const std::string& map_name)
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

        EXPECT_THROW(row_metadata.column(columns.size()),
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
    template<typename... Params>
    void check_partition_argument_index(std::string sql,
                                        std::shared_ptr<int32_t> expected_index,
                                        Params&&... arguments)
    {
        auto& sql_service = client.get_sql();
        EXPECT_EQ(sql_service.partition_argument_index_cache_->get(sql),
                  nullptr);
        sql_service.execute(sql, std::forward<Params>(arguments)...).get();

        if (expected_index == nullptr) {
            EXPECT_EQ(sql_service.partition_argument_index_cache_->get(sql),
                      nullptr);
        } else {
            ASSERT_NE(sql_service.partition_argument_index_cache_->get(sql),
                      nullptr);
            EXPECT_EQ(*sql_service.partition_argument_index_cache_->get(sql),
                      *expected_index);
        }
    }

    void check_partition_argument_index(sql::sql_statement statement,
                                        std::shared_ptr<int32_t> expected_index)
    {
        auto& sql_service = client.get_sql();
        EXPECT_EQ(
          sql_service.partition_argument_index_cache_->get(statement.sql()),
          nullptr);
        sql_service.execute(statement).get();

        if (expected_index == nullptr) {
            EXPECT_EQ(
              sql_service.partition_argument_index_cache_->get(statement.sql()),
              nullptr);
        } else {
            ASSERT_NE(
              sql_service.partition_argument_index_cache_->get(statement.sql()),
              nullptr);
            EXPECT_EQ(*sql_service.partition_argument_index_cache_->get(
                        statement.sql()),
                      *expected_index);
        }
    }

    int32_t get_partition_owner_index(int64_t key)
    {
        std::vector<member> members = client.get_cluster().get_members();
        spi::ClientContext clientContext(client);
        auto partition_id =
          clientContext.get_partition_service().get_partition_id(
            clientContext.get_serialization_service().to_data(key));
        auto owner = clientContext.get_partition_service().get_partition_owner(
          partition_id);

        for (size_t i = 0; i < members.size(); i++) {
            if (members[i].get_uuid() == owner) {
                return i;
            }
        }

        EXPECT_TRUE(false) << "Partition Owner not found for key: " << key;
        return 0;
    }

    void test_query_for_routing(std::string sql, int32_t keyCount)
    {

        std::vector<int32_t> member_2_instance_mapping =
          prepare_member_to_instance_mapping();
        int32_t members_size = client.get_cluster().get_members().size();
        // warm up cache
        client.get_sql().execute(sql, 0).get();

        // collect pre-execution metrics
        std::vector<uint64_t> expected_counts(members_size);
        for (size_t i = 0; i < expected_counts.size(); i++) {
            expected_counts[i] =
              get_direct_imap_queries_executed(member_2_instance_mapping[i]);
        }

        // run queries
        for (int64_t i = 1; i < keyCount; i++) {
            client.get_sql().execute(sql, i).get();
            expected_counts[get_partition_owner_index(i)]++;
        }

        // assert
        std::vector<uint64_t> actual_counts(members_size);

        for (size_t i = 0; i < actual_counts.size(); i++) {
            actual_counts[i] =
              get_direct_imap_queries_executed(member_2_instance_mapping[i]);
        }

        ASSERT_EQ(expected_counts, actual_counts);
    }
    
};

std::unique_ptr<HazelcastServerFactory> SqlTest::server_factory_;
std::unique_ptr<HazelcastServer> SqlTest::member_;
std::unique_ptr<HazelcastServer> SqlTest::member2_;
std::unique_ptr<HazelcastServer> SqlTest::member3_;

template<>
struct generator<test::student>
{
    test::student operator()()
    {
        return test::student{ int64_t(rand() % 1000),
                              float(rand() % 1000) * 2.0f };
    }
};


std::string
printer(const testing::TestParamInfo<iterator_type>& type)
{
    switch (type.param) {
        case iterator_type::page_iterator:
            return "page_iterator";
        case iterator_type::page_iterator_sync:
            return "page_iterator_sync";
        case iterator_type::row_iterator_sync:
            return "row_iterator_sync";
    }

    return "unknown";
}

INSTANTIATE_TEST_SUITE_P(SqlTestWithDifferentIterators,
                         SqlTest,
                         ::testing::Values(iterator_type::page_iterator,
                                           iterator_type::page_iterator_sync,
                                           iterator_type::row_iterator_sync),
                         printer);

TEST_F(SqlTest, test_hazelcast_exception)
{
    create_mapping();
    (void)populate_map(map);

    auto result_future = client.get_sql().execute("wrong-query");

    try {
        result_future.get();
        FAIL();
    } catch (const sql::hazelcast_sql_exception& ex) {
        auto uuid_str = boost::uuids::to_string(ex.originating_member_id());

        ASSERT_TRUE(uuid_str == member_->get_member().uuid ||
                    uuid_str == member2_->get_member().uuid ||
                    uuid_str == member3_->get_member().uuid);
    }
}

TEST_F(SqlTest, try_to_execute_on_closed_client)
{
    client.shutdown().get();
    EXPECT_THROW(select_all(), sql::hazelcast_sql_exception);
}

TEST_F(SqlTest, sql_page_column_count)
{
    create_mapping();
    (void)populate_map(map);

    ASSERT_EQ(select_all()->iterator().next().get()->column_count(), 2);
}

TEST_F(SqlTest, sql_page_column_types)
{
    create_mapping("VARCHAR");
    (void)populate_map<std::string>(map);

    auto result = select_all();

    auto types = result->iterator().next().get()->column_types();

    ASSERT_EQ(types.at(0), hazelcast::client::sql::sql_column_type::integer);
    ASSERT_EQ(types.at(1), hazelcast::client::sql::sql_column_type::varchar);
}

TEST_F(SqlTest, wrong_syntax)
{
    auto result_f =
      client.get_sql().execute("WRONG SYNTAX this should generate error");

    EXPECT_THROW(result_f.get(), sql::hazelcast_sql_exception);
}

TEST_F(SqlTest, row_metadata_on_non_select_query)
{
    // `DELETE` query is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping();
    (void)populate_map(map);

    auto result =
      client.get_sql()
        .execute(
          (boost::format("DELETE FROM %1% WHERE this < 18") % map_name).str())
        .get();

    EXPECT_THROW(result->row_metadata(), exception::illegal_state);
}

TEST_F(SqlTest, calling_iterator_next_consecutively)
{
    create_mapping();
    (void)populate_map(map, 100);

    auto handler = [this](){
        sql::sql_statement statement{ client,
                        (boost::format(
                                R"(
                SELECT * FROM %1%
            )") % map_name).str() };

        statement.cursor_buffer_size(10);
        auto result = client.get_sql().execute(statement).get();

        auto itr = result->iterator();

        auto p_1 =
          itr.next().get(); // Doesn't block since first page is already loaded.
        auto p_2 = itr.next();

        if (!p_2.has_value()) {
            EXPECT_THROW(itr.next(), exception::illegal_access);
            p_2.get();

            return true;
        } else {
            p_2.get();

            return false;
        }
    };

    ASSERT_TRUE_EVENTUALLY(handler());
}

TEST_F(SqlTest, calling_next_after_last_page_is_retrieved)
{
    create_mapping();
    (void)populate_map(map, 100);

    sql::sql_statement statement{ client,
                                  (boost::format(
                                     R"(
                    SELECT * FROM %1%
                )") % map_name)
                                    .str() };

    statement.cursor_buffer_size(10);

    auto result = client.get_sql().execute(statement).get();

    auto itr = result->iterator();

    while (itr.has_next())
        itr.next().get();

    EXPECT_THROW(itr.next(), exception::no_such_element);
}

TEST_F(SqlTest, simple)
{
    // `VALUES` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    sql::sql_statement statement(client, R"sql(
        SELECT * FROM (VALUES ('foo', 'bar'), (NULL, 'hello')) AS X(col1, col2)
    )sql");

    auto &service = client.get_sql();
    auto result = service.execute(statement).get();

    ASSERT_TRUE(result->row_set());
    EXPECT_EQ(-1, result->update_count());
    ASSERT_NO_THROW(result->row_metadata());
    ASSERT_EQ(2, result->row_metadata().columns().size());
    auto& column0 = result->row_metadata().columns()[0];
    EXPECT_EQ("col1", column0.name);
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column0.type);
    EXPECT_TRUE(column0.nullable);
    auto& column1 = result->row_metadata().columns()[1];
    EXPECT_EQ("col2", column1.name);
    EXPECT_EQ(hazelcast::client::sql::sql_column_type::varchar, column1.type);
    EXPECT_FALSE(column1.nullable);

    auto page_it = result->iterator();
    auto const& page = page_it.next().get();
    ASSERT_TRUE(page);
    auto& rows = page->rows();
    EXPECT_EQ(2, rows.size());
    EXPECT_EQ("foo", rows[0].get_object<std::string>(0).value());
    EXPECT_EQ("bar", rows[0].get_object<std::string>(1).value());
    EXPECT_FALSE(rows[1].get_object<std::string>(0).has_value());
    EXPECT_EQ("hello", rows[1].get_object<std::string>(1).value());
}

TEST_F(SqlTest, rows_can_be_used_even_after_the_result_is_destroyed)
{
    // `VALUES` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    sql::sql_statement statement(client, R"sql(
        SELECT * FROM (VALUES ('foo', 'bar'), (NULL, 'hello')) AS X(col1, col2)
    )sql");

    auto &service = client.get_sql();

    std::shared_ptr<sql::sql_page> page;

    {
        auto result = service.execute(statement).get();

        ASSERT_TRUE(result->row_set());

        page = result->iterator().next().get();
    }
    // result is destroyed at this point

    ASSERT_TRUE(page);
    auto& rows = page->rows();
    EXPECT_EQ(2, rows.size());
    EXPECT_EQ("foo", rows[0].get_object<std::string>(0).value());
    EXPECT_EQ("bar", rows[0].get_object<std::string>(1).value());
    EXPECT_FALSE(rows[1].get_object<std::string>(0).has_value());
    EXPECT_EQ("hello", rows[1].get_object<std::string>(1).value());
}

TEST_F(SqlTest, sql_result_public_apis_should_throw_after_close)
{
    // `TABLE` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    std::shared_ptr<sql::sql_result::page_iterator> it;
    {
        auto result = client.get_sql()
                        .execute("SELECT * from TABLE(generate_stream(1))")
                        .get();

        ASSERT_TRUE(result->row_set());

        it = std::make_shared<sql::sql_result::page_iterator>(
          result->iterator());

        result->close().get();
    }

    auto execution_1 = [&it]() { it->next().get(); };
    auto execution_2 = [&it]() { it->has_next(); };
    auto handler = [](const sql::hazelcast_sql_exception& e) {
        ASSERT_EQ(
          static_cast<int32_t>(sql::impl::sql_error_code::CANCELLED_BY_USER),
          e.code());
    };

    EXPECT_THROW_FN(execution_1(), sql::hazelcast_sql_exception, handler);
    EXPECT_THROW_FN(execution_2(), sql::hazelcast_sql_exception, handler);
}

TEST_F(SqlTest, statement_with_params)
{
    // `SELECT` query without FROM clause is not supported before 5,0,0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    auto &service = client.get_sql();
    auto result = service
                    .execute("SELECT CAST(? AS VARCHAR), CAST(? AS VARCHAR)",
                             123456,
                             -42.73)
                    .get();

    ASSERT_TRUE(result->row_set());
    EXPECT_EQ(-1, result->update_count());

    auto page = result->iterator().next().get();
    ASSERT_TRUE(page);

    auto& rows = page->rows();
    EXPECT_EQ(1, rows.size());
    EXPECT_EQ("123456", rows[0].get_object<std::string>(0).value());
    EXPECT_EQ("-42.73", rows[0].get_object<std::string>(1).value());
}

// These tests ported from python-client, sql_test.py
TEST_P(SqlTest, test_execute)
{
    create_mapping();
    auto expecteds = populate_map(map, 11);

    auto result = select_all();

    for_each_row(result,
                 assert_row_count{ expecteds.size() },
                 assert_entries_equal<int>{ expecteds });
}

TEST_P(SqlTest, test_execute_with_params)
{
    create_mapping();
    auto expecteds = populate_map(map, 13);

    auto result =
      client.get_sql()
        .execute(
          (boost::format("SELECT * FROM %1% WHERE this > ?") % map_name).str(),
          6)
        .get();

    auto expected_row_count =
      count_if(begin(expecteds),
               end(expecteds),
               [](const std::pair<int, int>& p) { return p.second > 6; });

    for_each_row(result,
                 assert_row_count(expected_row_count),
                 assert_entries_equal<int>{ expecteds });
}

TEST_F(SqlTest, test_execute_with_mismatched_params_when_sql_has_more)
{
    create_mapping();
    (void)populate_map(map);

    auto execution = client.get_sql().execute(
      (boost::format("SELECT * FROM %1% WHERE __key > ? AND this > ?") %
       map_name)
        .str(),
      5);

    auto handler =
      [](const hazelcast::client::sql::hazelcast_sql_exception& e) {
          EXPECT_EQ(e.code(), int(sql::impl::sql_error_code::DATA_EXCEPTION));
      };

    EXPECT_THROW_FN(execution.get(),
                    hazelcast::client::sql::hazelcast_sql_exception,
                    handler);
}

TEST_F(SqlTest, test_execute_with_mismatched_params_when_params_has_more)
{
    create_mapping();
    (void)populate_map(map);

    auto execution = client.get_sql().execute(
      (boost::format("SELECT * FROM %1% WHERE this > ?") % map_name).str(),
      5,
      6);

    auto handler =
      [](const hazelcast::client::sql::hazelcast_sql_exception& e) {
          EXPECT_EQ(e.code(), int(sql::impl::sql_error_code::DATA_EXCEPTION));
      };

    EXPECT_THROW_FN(execution.get(),
                    hazelcast::client::sql::hazelcast_sql_exception,
                    handler);
}

TEST_P(SqlTest, test_execute_statement)
{
    static constexpr int N_ENTRIES = 12;

    create_mapping("VARCHAR");
    auto expecteds = populate_map<std::string>(map, N_ENTRIES);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    auto result = client.get_sql().execute(statement).get();

    for_each_row(result,
                 assert_row_count(N_ENTRIES),
                 assert_entries_equal<std::string>{ expecteds });
}

TEST_P(SqlTest, test_execute_statement_with_params)
{
    create_mapping_for_student();

    auto expecteds = populate_map<test::student>(map, 20);

    sql::sql_statement statement{
        client,
        (boost::format("SELECT __key, age, height "
                       "FROM %1% "
                       "WHERE height = CAST(? AS REAL) OR age = ?") %
         map_name)
          .str()
    };

    auto height_param = expecteds.begin()->second.height;
    auto age_param = (++expecteds.begin())->second.age;

    statement.set_parameters(height_param, age_param);

    auto expected_row_counts = count_if(
      begin(expecteds),
      end(expecteds),
      [&](const std::pair<int, test::student>& p) {
          return p.second.height == height_param || p.second.age == age_param;
      });

    auto result = client.get_sql().execute(statement).get();

    for_each_row(result,
                 assert_row_count(expected_row_counts),
                 [&expecteds](const sql::sql_page::sql_row& row) {
                     auto idx = row.get_object<int>(0);
                     EXPECT_TRUE(idx.has_value());

                     const auto& expected = expecteds.at(*idx);

                     auto age = row.get_object<int64_t>(1);
                     EXPECT_TRUE(age.has_value());
                     EXPECT_EQ(*age, expected.age);

                     auto height = row.get_object<float>(2);
                     EXPECT_TRUE(height.has_value());
                     EXPECT_EQ(row.get_object<float>(2), expected.height);
                 });
}

TEST_F(SqlTest, test_execute_statement_with_params_after_clear_parameters)
{
    create_mapping_for_student();

    auto expecteds = populate_map<test::student>(map, 20);

    sql::sql_statement statement{
        client,
        (boost::format("SELECT __key, age, height "
                       "FROM %1% "
                       "WHERE height = CAST(? AS REAL) OR age = ?") %
         map_name)
          .str()
    };

    auto height_param = expecteds.begin()->second.height;
    auto age_param = (++expecteds.begin())->second.age;

    statement.set_parameters(height_param);
    statement.clear_parameters();
    statement.set_parameters(height_param, age_param);

    EXPECT_NO_THROW(client.get_sql().execute(statement).get());
}

TEST_F(SqlTest, test_execute_statement_with_mismatched_params_when_sql_has_more)
{
    create_mapping();
    (void)populate_map(map);

    sql::sql_statement statement{
        client,
        (boost::format("SELECT * FROM %1% WHERE __key > ? AND this > ?") %
         map_name)
          .str()
    };

    statement.set_parameters(5);
    auto execution = client.get_sql().execute(statement);

    auto handler =
      [](const hazelcast::client::sql::hazelcast_sql_exception& e) {
          EXPECT_EQ(e.code(), int(sql::impl::sql_error_code::DATA_EXCEPTION));
      };

    EXPECT_THROW_FN(execution.get(),
                    hazelcast::client::sql::hazelcast_sql_exception,
                    handler);
}

TEST_F(SqlTest,
       test_execute_statement_with_mismatched_params_when_params_has_more)
{
    create_mapping();
    (void)populate_map(map);

    sql::sql_statement statement{
        client,
        (boost::format("SELECT * FROM %1% WHERE this > ?") % map_name).str()
    };

    statement.set_parameters(5, 6);
    auto execution = client.get_sql().execute(statement);

    auto handler =
      [](const hazelcast::client::sql::hazelcast_sql_exception& e) {
          EXPECT_EQ(e.code(), int(sql::impl::sql_error_code::DATA_EXCEPTION));
      };

    EXPECT_THROW_FN(execution.get(),
                    hazelcast::client::sql::hazelcast_sql_exception,
                    handler);
}

TEST_P(SqlTest, test_execute_statement_with_timeout)
{
    create_mapping_for_student();
    auto expecteds = populate_map<test::student>(map, 100);

    sql::sql_statement statement{
        client,
        (boost::format("SELECT __key, height FROM %1% WHERE height < 100") %
         map_name)
          .str()
    };

    statement.timeout(std::chrono::milliseconds{ 5000 });

    auto expected_row_count =
      count_if(begin(expecteds),
               end(expecteds),
               [](const std::pair<int, test::student>& p) {
                   return p.second.height < 100.0f;
               });

    auto result = client.get_sql().execute(statement).get();

    for_each_row(result,
                 assert_row_count(expected_row_count),
                 [&expecteds](const sql::sql_page::sql_row& row) {
                     auto idx = row.get_object<int>(0);

                     EXPECT_TRUE(idx.has_value());
                     auto height = row.get_object<float>(1);
                     EXPECT_TRUE(height.has_value());
                     EXPECT_EQ(expecteds.at(*idx).height, *height);
                 });
}

TEST_F(SqlTest, test_statement_with_wrong_timeout)
{
    sql::sql_statement statement{ client, "non_null_query" };

    EXPECT_THROW(statement.timeout(std::chrono::milliseconds{ -500 }),
                 exception::illegal_argument);
}

TEST_F(SqlTest, test_statement_with_empty_sql)
{
    auto fn = [this]() { sql::sql_statement statement{ client, "" }; };

    EXPECT_THROW(fn(), exception::illegal_argument);
}

TEST_F(SqlTest, test_execute_with_cursor_buffer_size)
{
    static constexpr int CURSOR_BUFFER_SIZE = 3;

    create_mapping_for_student();
    (void)populate_map<test::student>(map, 48);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    statement.cursor_buffer_size(CURSOR_BUFFER_SIZE);

    auto result = client.get_sql().execute(statement).get();

    for (auto itr = result->iterator(); itr.has_next();) {
        EXPECT_EQ(itr.next().get()->row_count(), CURSOR_BUFFER_SIZE);
    }
}

TEST_F(SqlTest, test_execute_with_schema)
{
    create_mapping_for_student();
    (void)populate_map<test::student>(map, 48);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    statement.schema(std::string{ "nothing" });

    ASSERT_NO_THROW(client.get_sql().execute(statement).get());
}

TEST_F(SqlTest, test_execute_with_expected_result_type_as_rows)
{
    create_mapping_for_student();
    (void)populate_map<test::student>(map);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    statement.expected_result_type(sql::sql_expected_result_type::rows);

    EXPECT_NO_THROW(client.get_sql().execute(statement).get());
}

TEST_F(SqlTest, test_execute_with_expected_result_type_as_rows_type_mismatch)
{
    create_mapping_for_student();
    (void)populate_map<test::student>(map);

    sql::sql_statement statement{
        client, (boost::format("DELETE FROM %1%") % map_name).str()
    };

    statement.expected_result_type(sql::sql_expected_result_type::rows);

    EXPECT_THROW(client.get_sql().execute(statement).get(),
                 sql::hazelcast_sql_exception);
}

TEST_F(SqlTest, test_execute_with_expected_result_type_as_update_count)
{
    // `DELETE` query is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping_for_student();
    (void)populate_map<test::student>(map);

    sql::sql_statement statement{
        client, (boost::format("DELETE FROM %1%") % map_name).str()
    };

    statement.expected_result_type(sql::sql_expected_result_type::update_count);

    EXPECT_NO_THROW(client.get_sql().execute(statement).get());
}

TEST_F(SqlTest,
       test_execute_with_expected_result_type_as_update_count_type_mismatch)
{
    create_mapping_for_student();
    (void)populate_map<test::student>(map);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    statement.expected_result_type(sql::sql_expected_result_type::update_count);

    EXPECT_THROW(client.get_sql().execute(statement).get(),
                 sql::hazelcast_sql_exception);
}

TEST_F(SqlTest, test_execute_with_expected_result_type_as_any)
{
    // `DELETE` query is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping_for_student();
    (void)populate_map<test::student>(map);

    sql::sql_statement statement_1{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    statement_1.expected_result_type(sql::sql_expected_result_type::any);
    EXPECT_NO_THROW(client.get_sql().execute(statement_1).get());

    sql::sql_statement statement_2{
        client, (boost::format("DELETE FROM %1%") % map_name).str()
    };

    statement_2.expected_result_type(sql::sql_expected_result_type::any);

    EXPECT_NO_THROW(client.get_sql().execute(statement_2).get());
}

TEST_F(SqlTest, test_is_row_set_when_row_is_set)
{
    create_mapping_for_student();
    auto expecteds = populate_map<test::student>(map, 100);

    sql::sql_statement statement{
        client, (boost::format("SELECT * FROM %1%") % map_name).str()
    };

    auto result = client.get_sql().execute(statement).get();

    ASSERT_TRUE(result->row_set());
    ASSERT_EQ(result->update_count(), -1);
}

TEST_F(SqlTest, test_is_row_set_when_there_is_no_update)
{
    // `UPDATE` query is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping_for_student();
    auto expecteds = populate_map<test::student>(map, 100);

    sql::sql_statement statement{
        client,
        (boost::format("UPDATE %1% SET age = 4 WHERE FALSE") % map_name).str()
    };

    auto result = client.get_sql().execute(statement).get();

    ASSERT_EQ(result->update_count(), 0);
}

TEST_F(SqlTest, test_null)
{
    create_mapping();
    (void)populate_map(map, 50);

    auto result =
      client.get_sql()
        .execute(
          (boost::format("SELECT __key, NULL AS this FROM %1%") % map_name)
            .str())
        .get();

    auto type = result->row_metadata().columns().back().type;

    EXPECT_EQ(type, sql::sql_column_type::null);
}

TEST_P(SqlTest, test_object)
{
    create_mapping_for_student();
    auto expecteds = populate_map<test::student>(map);

    auto result =
      client.get_sql()
        .execute(
          (boost::format("SELECT __key, this FROM %1%") % map_name).str())
        .get();

    for_each_row(result,
                 assert_row_count{ expecteds.size() },
                 assert_entries_equal<test::student>{ expecteds });
}

TEST_F(SqlTest, test_null_only_column)
{
    create_mapping();
    (void)populate_map(map);

    auto result =
      client.get_sql()
        .execute((boost::format(
                    "SELECT __key, CAST(NULL AS INTEGER) as this FROM %1%") %
                  map_name)
                   .str())
        .get();

    auto col_type = result->row_metadata().columns().back().type;
    EXPECT_EQ(col_type, sql::sql_column_type::integer);

    auto value =
      result->iterator().next().get()->rows().at(0).get_object<int>(1);
    EXPECT_FALSE(value.has_value());
}

TEST_P(SqlTest, test_json)
{
    // `JSON` is not supported before 5.1.0
    if (cluster_version() < member::version{ 5, 1, 0 })
        GTEST_SKIP();

    create_mapping("JSON");
    auto expecteds = populate_map<hazelcast_json_value>(map, 50);

    auto result = select_all();

    for_each_row(result,
                 assert_row_count(expecteds.size()),
                 assert_entries_equal<hazelcast_json_value>{ expecteds });
}

TEST_P(SqlTest, test_streaming_sql_query)
{
    // `TABLE` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    auto result = client.get_sql()
                    .execute("SELECT * FROM TABLE(generate_stream(100))")
                    .get();

    for_each_row_until(200, result);
}

TEST_P(SqlTest, test_date)
{
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping("DATE");

    auto expecteds = populate_map_via_rc<local_date>(
      [](const local_date& date) {
          return boost::str(
            boost::format("java.time.LocalDate.of(%1%, %2%, %3%)") %
            int(date.year) % int(date.month) % int(date.day_of_month));
      },
      map_name,
      server_factory_->get_cluster_id());

    auto result = select_all();

    for_each_row(result,
                 assert_row_count(expecteds.size()),
                 assert_entries_equal<local_date>{ expecteds });
}

TEST_P(SqlTest, test_time)
{
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping("TIME");

    auto expecteds = populate_map_via_rc<local_time>(
      [](const local_time& time) {
          return boost::str(
            boost::format("java.time.LocalTime.of(%1%, %2%, %3%, %4%)") %
            int(time.hours) % int(time.minutes) % int(time.seconds) %
            int(time.nanos));
      },
      map_name,
      server_factory_->get_cluster_id());

    auto result = select_all();

    for_each_row(result,
                 assert_row_count(expecteds.size()),
                 assert_entries_equal<local_time>{ expecteds });
}

TEST_P(SqlTest, test_timestamp)
{
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping("TIMESTAMP");

    auto expecteds = populate_map_via_rc<local_date_time>(
      [](const local_date_time& dt) {
          return boost::str(
            boost::format(
              "java.time.LocalDateTime.of(%1%, %2%, %3%, %4%, %5%, %6%, %7%)") %
            dt.date.year % int(dt.date.month) % int(dt.date.day_of_month) %
            int(dt.time.hours) % int(dt.time.minutes) % int(dt.time.seconds) %
            int(dt.time.nanos));
      },
      map_name,
      server_factory_->get_cluster_id());

    auto result = select_all();

    for_each_row(result,
                 assert_row_count(expecteds.size()),
                 assert_entries_equal<local_date_time>{ expecteds });
}

TEST_P(SqlTest, test_timestamp_with_timezone)
{
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    create_mapping("TIMESTAMP WITH TIME ZONE");

    auto expecteds = populate_map_via_rc<offset_date_time>(
      [](const offset_date_time& dt) {
          return boost::str(
            boost::format("java.time.OffsetDateTime.of(%1%, %2%, %3%, %4%, "
                          "%5%, %6%, %7%, java.time.ZoneOffset.ofHours(%8%))") %
            dt.date_time.date.year % int(dt.date_time.date.month) %
            int(dt.date_time.date.day_of_month) % int(dt.date_time.time.hours) %
            int(dt.date_time.time.minutes) % int(dt.date_time.time.seconds) %
            int(dt.date_time.time.nanos) % (dt.zone_offset_in_seconds / 3600));
      },
      map_name,
      server_factory_->get_cluster_id());

    auto result = select_all();

    for_each_row(result,
                 assert_row_count(expecteds.size()),
                 assert_entries_equal<offset_date_time>{ expecteds });
}

TEST_F(SqlTest, exception)
{
    sql::sql_service service = client.get_sql();
    EXPECT_THROW(service.execute("FOO BAR", 42).get(),
                 hazelcast::client::sql::hazelcast_sql_exception);
}

TEST_F(SqlTest, find_with_page_sync_iterator)
{
    create_mapping();
    auto numbers = populate_map(map, 500);

    auto searchee = numbers[numbers.size() / 2];
    auto result = select_all();

    auto found_page_itr = std::find_if(
        result->pbegin(),
        result->pend(),
        [searchee](std::shared_ptr<sql::sql_page> page){
            return find_if(
                begin(page->rows()),
                end(page->rows()),
                [searchee](const sql::sql_page::sql_row& row){
                    return row.get_object<int>(1).value() == searchee;
                }
            ) != end(page->rows());
        }
    );

    ASSERT_NE(found_page_itr, result->pend());

    bool exist = any_of(
        begin(found_page_itr->rows()),
        end(found_page_itr->rows()),
        [searchee](const sql::sql_page::sql_row& row){
            return row.get_object<int>(1).value() == searchee;
        }
    );

    ASSERT_TRUE(exist);
}

TEST_F(SqlTest, find_with_row_sync_iterator)
{
    create_mapping();
    auto numbers = populate_map(map, 500);

    auto searchee = numbers[numbers.size() / 2];
    auto result = select_all();

    auto found_row_itr = std::find_if(
        begin(*result),
        end(*result),
        [searchee](const sql::sql_page::sql_row& row){
            return row.get_object<int>(1).value() == searchee;
        }
    );

    ASSERT_NE(found_row_itr, end(*result));
    ASSERT_EQ(found_row_itr->get_object<int>(1).value(), searchee);
}

TEST_F(SqlTest, timeout_for_page_iterator_sync)
{
    // `TABLE` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    // `generate_stream(1)` generates a row per seconds, so it will guaranteed
    // that it will timeout
    auto result =
      client.get_sql().execute("SELECT * FROM TABLE(generate_stream(1))").get();

    auto statement = [&result](){
        auto it = result->pbegin(std::chrono::milliseconds{ 1 });
        ++it;
        ++it;
    };

    ASSERT_THROW(statement(), hazelcast::client::exception::no_such_element);
}

TEST_F(SqlTest, timeout_for_row_iterator_sync)
{
    // `TABLE` clause is not supported before 5.0.0
    if (cluster_version() < member::version{ 5, 0, 0 })
        GTEST_SKIP();

    // `generate_stream(1)` generates a row per seconds, so it will guaranteed
    // that it will timeout
    auto result =
      client.get_sql().execute("SELECT * FROM TABLE(generate_stream(1))").get();

    auto statement = [&result](){
        auto it = result->begin(std::chrono::milliseconds{ 1 });
        ++it;
        ++it;
    };

    ASSERT_THROW(statement(), hazelcast::client::exception::no_such_element);
}

// ported from Java SqlBasicClientTest.testSelect
TEST_F(SqlTest, select)
{
    sql::sql_service service = client.get_sql();

    constexpr std::size_t DATA_SET_SIZE = 4096;
    std::unordered_map<portable_pojo_key, portable_pojo> entries(DATA_SET_SIZE);
    for (std::size_t i = 0; i < DATA_SET_SIZE; ++i) {
        entries[key(i)] = value(i);
    }

    map->put_all(entries).get();

    ASSERT_EQ(DATA_SET_SIZE, map->size().get());

    // Mapping is not supported before 5.0.0
    if (cluster_version() >= member::version{ 5, 0, 0 }) {
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
           serialization::hz_serializer<
             portable_pojo_key>::PORTABLE_FACTORY_ID %
           serialization::hz_serializer<
             portable_pojo_key>::PORTABLE_KEY_CLASS_ID %
           serialization::hz_serializer<portable_pojo>::PORTABLE_FACTORY_ID %
           serialization::hz_serializer<portable_pojo>::PORTABLE_VALUE_CLASS_ID)
            .str();

        auto result = service.execute(sql).get();
    }

    using namespace sql;

    auto res = query(map_name);

    auto const& row_metadata = res->row_metadata();
    check_row_metada(row_metadata);

    std::unordered_set<int64_t> unique_keys;

    for (auto itr = res->iterator(); itr.has_next();) {
        auto page = itr.next().get();
        for (const auto& row : page->rows()) {
            ASSERT_EQ(row_metadata, res->row_metadata());

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

            EXPECT_THROW(row.get_object<int>(-1),
                         exception::index_out_of_bounds);
            EXPECT_THROW(row.get_object<int>(row.row_metadata().column_count()),
                         exception::index_out_of_bounds);
            EXPECT_THROW(row.get_object<int>("unknown_field"),
                         exception::illegal_argument);
        }
    }

    EXPECT_THROW(res->iterator(), exception::illegal_state);

    ASSERT_EQ(DATA_SET_SIZE, unique_keys.size());

    EXPECT_NO_THROW(res->close().get());

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

TEST_F(SqlTest, test_partition_based_routing_simple_type_test)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping("VARCHAR");

    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (__key, this) VALUES (?, ?)") % map_name)
        .str(),
      std::make_shared<int32_t>(0),
      1,
      "value");
    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (this, __key) VALUES (?, ?)") % map_name)
        .str(),
      std::make_shared<int32_t>(1),
      "value",
      2);
    // no dynamic argument
    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (this, __key) VALUES ('value', 3)") %
       map_name)
        .str(),
      nullptr);
    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (this, __key) "
                     "VALUES ('value', 4), ('value', 5)") %
       map_name)
        .str(),
      nullptr);
    // has dynamic argument, but multiple rows
    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (this, __key) VALUES (?, ?), (?, ?)") %
       map_name)
        .str(),
      nullptr,
      "value",
      6,
      "value",
      7);
}

TEST_F(SqlTest, test_partition_based_routing_with_statements)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping("VARCHAR");

    sql::sql_statement statement1(
      client,
      (boost::format("INSERT INTO %1% (__key, this) VALUES (?, ?)") % map_name)
        .str());
    statement1.add_parameter(1);
    statement1.add_parameter("value");
    check_partition_argument_index(statement1, std::make_shared<int32_t>(0));
    EXPECT_EQ(*statement1.partition_argument_index(), 0);

    sql::sql_statement statement2(
      client,
      (boost::format("INSERT INTO %1% (this, __key) VALUES (?, ?)") % map_name)
        .str());
    statement2.add_parameter("value");
    statement2.add_parameter(2);

    check_partition_argument_index(statement2, std::make_shared<int32_t>(1));
    EXPECT_EQ(*statement2.partition_argument_index(), 1);

    sql::sql_statement statement3(
      client,
      (boost::format("INSERT INTO %1% (this, __key) VALUES ('value', 3)") %
       map_name)
        .str());

    // no dynamic argument
    check_partition_argument_index(statement3, nullptr);
    EXPECT_EQ(*statement3.partition_argument_index(), -1);

    sql::sql_statement statement4(
      client,
      (boost::format("INSERT INTO %1% (this, __key) "
                     "VALUES ('value', 4), ('value', 5)") %
       map_name)
        .str());

    check_partition_argument_index(statement4, nullptr);
    EXPECT_EQ(*statement4.partition_argument_index(), -1);

    sql::sql_statement statement5(
      client,
      (boost::format("INSERT INTO %1% (this, __key) VALUES (?, ?), (?, ?)") %
       map_name)
        .str());
    statement5.add_parameter("value");
    statement5.add_parameter(6);
    statement5.add_parameter("value");
    statement5.add_parameter(7);

    // has dynamic argument, but multiple rows
    check_partition_argument_index(statement5, nullptr);
    EXPECT_EQ(*statement5.partition_argument_index(), -1);
}

TEST_F(SqlTest, test_partition_based_routing)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();
    std::string test_map_name{ random_map_name() };
    auto test_map = client.get_map(test_map_name).get();

    create_mapping("VARCHAR");
    create_mapping("VARCHAR", test_map_name);

    check_partition_argument_index(
      (boost::format("SELECT * FROM %1% WHERE __key = ?") % map_name).str(),
      std::make_shared<int32_t>(0),
      1);

    check_partition_argument_index(
      (boost::format("UPDATE %1% SET this = ? WHERE __key = ?") % map_name)
        .str(),
      std::make_shared<int32_t>(1),
      "testVal",
      1);

    check_partition_argument_index(
      (boost::format("DELETE FROM %1% WHERE __key = ?") % map_name).str(),
      std::make_shared<int32_t>(0),
      1);

    check_partition_argument_index(
      (boost::format(
         "SELECT JSON_OBJECT(this : __key) FROM %1% WHERE __key = ?") %
       map_name)
        .str(),
      std::make_shared<int32_t>(0),
      1);

    check_partition_argument_index(
      (boost::format(
         "SELECT JSON_ARRAY(__key, this) FROM %1% WHERE __key = ?") %
       map_name)
        .str(),
      std::make_shared<int32_t>(0),
      1);

    // aggregation
    check_partition_argument_index(
      (boost::format(
         "SELECT JSON_OBJECTAGG(this : __key) FROM %1% WHERE __key = ?") %
       map_name)
        .str(),
      nullptr,
      1);
    check_partition_argument_index(
      (boost::format("SELECT SUM(__key) FROM %1% WHERE __key = ?") % map_name)
        .str(),
      nullptr,
      1);
    check_partition_argument_index(
      (boost::format("SELECT COUNT(*) FROM %1% WHERE __key = ?") % map_name)
        .str(),
      nullptr,
      1);

    // join
    check_partition_argument_index(
      (boost::format("SELECT * FROM %1% t1 JOIN %2% t2 ON t1.__key = t2.__key "
                     "WHERE t1.__key = ?") %
       map_name % test_map_name)
        .str(),
      nullptr,
      1);

    check_partition_argument_index(
      (boost::format("SELECT t1.*, t2.* FROM %1% t1 JOIN %2% t2 USING(__key) "
                     "WHERE t1.__key = ?") %
       map_name % test_map_name)
        .str(),
      nullptr,
      1);
}

TEST_F(SqlTest, test_partition_based_routing_complex_type_test)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();
    std::string custom_map_name{ random_map_name() };
    auto custom_map = client.get_map(custom_map_name).get();
    auto sql =
      (boost::format("CREATE OR REPLACE MAPPING %1% ("
                     "key BIGINT EXTERNAL NAME \"__key.key\", "
                     "this VARCHAR "
                     ") TYPE IMap OPTIONS( "
                     "'keyFormat'='portable'"
                     ", 'keyPortableFactoryId'='%2%'"
                     ", 'keyPortableClassId'='%3%'"
                     ", 'keyPortableClassVersion'='0'"
                     ", 'valueFormat'='varchar'"
                     ")") %
       custom_map_name %
       serialization::hz_serializer<portable_pojo_key>::PORTABLE_FACTORY_ID %
       serialization::hz_serializer<portable_pojo_key>::PORTABLE_KEY_CLASS_ID)
        .str();

    client.get_sql().execute(sql).get();

    // partition argument index not supported if `__key` isn't directly assigned
    // to
    check_partition_argument_index(
      (boost::format("INSERT INTO %1% (this, key) VALUES (?, ?)") %
       custom_map_name)
        .str(),
      nullptr,
      "value1",
      1);

    create_mapping_for_student_as_key();
    try {
        // this test case is here just for completeness to show that we cannot
        // support complex keys and partition argument
        check_partition_argument_index(
          (boost::format("INSERT INTO %1% (this, __key) VALUES (?, ?)") %
           map_name)
            .str(),
          nullptr,
          "value-1",
          student{ 2, 1.72 });
    } catch (exception::iexception& ie) {
        auto msg = ie.get_message();
        ASSERT_NE(
          std::string::npos,
          msg.find("Writing to top-level fields of type OBJECT not supported"));
    }
}

TEST_F(SqlTest, test_partition_based_routing_complex_key)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping_for_student_as_key();

    check_partition_argument_index(
      (boost::format("SELECT * FROM %1% WHERE __key = ?") % map_name).str(),
      std::make_shared<int32_t>(0),
      student{ 2, 1.72 });

    check_partition_argument_index(
      (boost::format("UPDATE %1% SET this = ? WHERE __key = ?") % map_name)
        .str(),
      std::make_shared<int32_t>(1),
      "testVal",
      student{ 2, 1.72 });

    check_partition_argument_index(
      (boost::format("DELETE FROM %1% WHERE __key = ?") % map_name).str(),
      std::make_shared<int32_t>(0),
      student{ 2, 1.72 });
}

TEST_F(SqlTest, test_routing_for_select)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping();
    test_query_for_routing(
      (boost::format("SELECT * FROM %1% WHERE __key = ?") % map_name).str(),
      100);
}

TEST_F(SqlTest, test_routing_for_insert)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping("VARCHAR");
    test_query_for_routing(
      (boost::format("INSERT INTO %1% (this, __key) VALUES ('testVal', ?)") %
       map_name)
        .str(),
      100);
}

TEST_F(SqlTest, test_routing_for_update)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping("VARCHAR");
    test_query_for_routing(
      (boost::format("UPDATE %1% SET this = 'testVal' WHERE __key = ?") %
       map_name)
        .str(),
      100);
}

TEST_F(SqlTest, test_routing_for_delete)
{
    if (cluster_version() < member::version{ 5, 3, 0 })
        GTEST_SKIP();

    create_mapping("VARCHAR");
    test_query_for_routing(
      (boost::format("DELETE FROM %1% WHERE __key = ?") % map_name).str(), 100);
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

template<typename K, typename V>
class read_optimized_lru_cache_for_testing
  : public sql::impl::read_optimized_lru_cache<K, V>
{

public:
    read_optimized_lru_cache_for_testing(int32_t capacity,
                                         int32_t cleanup_threshold)
      : sql::impl::read_optimized_lru_cache<K, V>(capacity, cleanup_threshold)
    {
    }

    int32_t get_cache_size() { return this->cache_.size(); }

    V get_cache_value(K k) { return this->cache_.get(k)->value_; }
};

class read_optimized_lru_cache_test : public ::testing::Test
{

public:
    read_optimized_lru_cache_for_testing<int32_t, int32_t> lru;
    read_optimized_lru_cache_test()
      : lru(2, 3)
    {
    }
};

TEST_F(read_optimized_lru_cache_test, construction_test)
{
    using cache_type = sql::impl::read_optimized_lru_cache<int32_t, int32_t>;
    std::shared_ptr<cache_type> lru_cache;

    EXPECT_THROW(std::make_shared<cache_type>(0, 0),
                 exception::illegal_argument);

    EXPECT_THROW(std::make_shared<cache_type>(10, 5),
                 exception::illegal_argument);

    EXPECT_NO_THROW(std::make_shared<cache_type>(10, 15));
}

TEST_F(read_optimized_lru_cache_test, put_test)
{
    EXPECT_EQ(0, lru.get_cache_size());
    lru.put(1, std::make_shared<int32_t>(10));
    EXPECT_EQ(1, lru.get_cache_size());
    EXPECT_EQ(10, lru.get_cache_value(1));
}

TEST_F(read_optimized_lru_cache_test, get_test)
{
    EXPECT_EQ(0, lru.get_cache_size());
    EXPECT_EQ(nullptr, lru.get(1));
    lru.put(1, std::make_shared<int32_t>(10));
    EXPECT_EQ(10, *(lru.get(1)));
}

TEST_F(read_optimized_lru_cache_test, put_nullptr_test)
{
    EXPECT_EQ(0, lru.get_cache_size());
    EXPECT_THROW(lru.put(1, nullptr), client::exception::illegal_argument);
}

TEST_F(read_optimized_lru_cache_test, get_or_default_test)
{
    EXPECT_EQ(0, lru.get_cache_size());
    EXPECT_EQ(2, *(lru.get_or_default(1, std::make_shared<int32_t>(2))));

    lru.put(3, std::make_shared<int32_t>(10));

    EXPECT_EQ(10, *(lru.get_or_default(3, std::make_shared<int32_t>(2))));
}

TEST_F(read_optimized_lru_cache_test, put_and_remove_test)
{
    EXPECT_EQ(0, lru.get_cache_size());
    lru.put(1, std::make_shared<int32_t>(10));
    EXPECT_EQ(1, lru.get_cache_size());
    lru.remove(1);
    EXPECT_EQ(0, lru.get_cache_size());
}

TEST_F(read_optimized_lru_cache_test, eviction_test)
{
    lru.put(42, std::make_shared<int32_t>(42));
    // a little sleep to ensure the lastUsed timestamps are different even on a
    // very imprecise clock
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lru.put(43, std::make_shared<int32_t>(43));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lru.put(44, std::make_shared<int32_t>(44));
    EXPECT_EQ(3, lru.get_cache_size());

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lru.put(45, std::make_shared<int32_t>(45));
    EXPECT_EQ(2, lru.get_cache_size());
    EXPECT_EQ(44, lru.get_cache_value(44));
    EXPECT_EQ(45, lru.get_cache_value(45));

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lru.put(46, std::make_shared<int32_t>(46));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lru.get(44); // access makes the value the least recently used one

    lru.put(47, std::make_shared<int32_t>(47));
    EXPECT_EQ(2, lru.get_cache_size());
    EXPECT_EQ(44, lru.get_cache_value(44));
    EXPECT_EQ(47, lru.get_cache_value(47));
}

} // namespace test
} // namespace client
} // namespace hazelcast
