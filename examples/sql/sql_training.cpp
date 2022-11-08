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
#include <fstream>
#include <iterator>
#include <iomanip>

namespace case_1
{
  void run( hazelcast::client::hazelcast_client hz)
  {
    std::cout << std::string ( 50 , '=' ) << std::endl;
    std::cout << "case-1 - Read integers" << std::endl;
    std::cout << std::string ( 50 , '=' ) << std::endl;
      // populate the map with some data
      auto map = hz.get_map("integers").get();
      for (int i = 0; i < 100000; ++i)
          map->put(i, i).get();

      auto sql = hz.get_sql();
      // Create mapping for the integers. This needs to be done only once per map.
      auto result = sql
                      .execute(R"(
                  CREATE OR REPLACE MAPPING integers
                    TYPE IMap
                      OPTIONS (
                        'keyFormat' = 'int',
                        'valueFormat' = 'int'
                        )
                      )")
                      .get();

      result = sql.execute("SELECT * FROM integers").get();

      std::shared_ptr<hazelcast::client::sql::sql_page> page;
      int rows {};

      for (auto it = result->iterator(); it.has_next();)
      {
        auto page = it.next().get();
        rows += page->row_count();
      }

      std::cout << "There are " << rows << " rows." << std::endl;
  }
}

namespace case_2
{
  struct employee
  {
    std::string name;
    int age;

    friend inline std::ostream& operator<<(std::ostream& os , const employee& e)
    {
      return os << e.name << " " << e.age;
    }
  };

  using namespace hazelcast::client;

  struct employees_loader
  {
    hazelcast::client::sql::sql_service& sql;
    boost::optional<sql::sql_result::page_iterator> itr;

    bool has_next() const
    {
      return itr.has_value() && itr->has_next();
    }

    boost::future<std::vector<employee>> read_some()
    {
      if ( !itr.has_value() )
      {
        // Create mapping for the employees map. This needs to be done only once per map.
        sql.execute(R"(
                CREATE OR REPLACE MAPPING employees
                TYPE IMap
                OPTIONS (
                    'keyFormat' = 'int',
                    'valueFormat' = 'json'
                )
            )")
            .get();
        auto result = sql.execute(R"(
              SELECT JSON_VALUE(this, '$.name') AS name, JSON_VALUE(this, '$.age' RETURNING INT)  AS age
              FROM employees
          )")
          .get();
        
        itr = result->iterator();
      }

      if ( itr->has_next() )
      {
        return itr->next().then(
          [](boost::future<std::shared_ptr<sql::sql_page>> page_f){
            auto page = page_f.get();

            std::vector<employee> employees;

            transform(
              begin(page->rows()) ,
              end(page->rows()) ,
              back_inserter(employees) ,
              []( const sql::sql_page::sql_row& row ){
                return employee {
                  *row.get_object<std::string>("name") ,
                  *row.get_object<int>("age") ,
                };
              }
            );

            return employees;
          }
        );
      }
    }
  };

  struct modify_age
  {
    std::vector<employee> operator()( boost::future<std::vector<employee>> fut )
    {
      auto employees = fut.get();

      for (employee& e : employees)
        e.age += 5;
      
      return employees;
    }
  };

  struct print_to_file
  {
    std::string file_name;

    template<typename T>
    auto operator()(T future)
    {
      auto collection = future.get();
      std::ofstream ofs{ file_name };

      for (const auto& entry : collection)
        ofs << entry << "\n";
      
      return collection;
    }
  };

  struct print_to_stdout
  {
    template<typename T>
    auto operator()(T future)
    {
      auto collection = future.get();

      for (const auto& entry : collection)
        std::cout << std::setw( 6 ) << entry << std::endl;
      
      return collection;
    }
  };

  void run(hazelcast::client::hazelcast_client hz)
  {
    std::cout << std::string ( 50 , '=' ) << std::endl;
    std::cout << "case-2 - Continuations" << std::endl;
    std::cout << std::string ( 50 , '=' ) << std::endl;
    auto employees = hz.get_map("employees").get();

    std::vector<hazelcast_json_value> values
    {
      { R"({"name": "Alice", "age": 32})" } ,
      { R"({"name": "John", "age": 42})" } ,
      { R"({"name": "Jake", "age": 18})" } ,
      { R"({"name": "Mike", "age": 63})" } ,
      { R"({"name": "Jimmy", "age": 44})" } ,
      { R"({"name": "Chuck", "age": 71})" }
    };

    for (auto i = 0; i < values.size(); ++i)
      employees->put(i , values.at(i));

    employees_loader loader { hz.get_sql() };

    do
    {
      loader.read_some()
            .then( modify_age {} )
            .then( print_to_file { "employees.txt" } )
            .then( print_to_stdout {} )
            .get();
    }
    while( loader.has_next() );
  }
}

namespace case_3
{
  void run(hazelcast::client::hazelcast_client hz)
  {
    std::cout << std::string ( 50 , '=' ) << std::endl;
    std::cout << "case-3 - Stream processing" << std::endl;
    std::cout << std::string ( 50 , '=' ) << std::endl;
    auto sql = hz.get_sql();

    auto result = sql.execute("SELECT * FROM TABLE(generate_stream(1))").get();

    auto start = std::chrono::steady_clock::now();
    for (auto itr = result->iterator(); itr.has_next();)
    {
      auto result = itr.next().get();

      for (const auto& row : result->rows())
        std::cout << row.get_object<int64_t>(0) << std::endl;

      auto now = std::chrono::steady_clock::now();

      if ( std::chrono::duration_cast<std::chrono::seconds>( now - start ).count() > 10 )
        break;
    }

    std::cout << std::string ( 50 , '=' ) << std::endl;
  }
}

int
main()
{
    using namespace hazelcast::client::sql;

    auto hz = hazelcast::new_client().get();

    case_1::run(hz);
    case_2::run(hz);
    case_3::run(hz);
}
