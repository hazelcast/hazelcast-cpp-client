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

#pragma once

#include "hazelcast/client/serialization/generic_record_builder.h"

#include "compact_test_base.h"
#include "../TestHelperFunctions.h"

namespace hazelcast {
namespace client {
namespace test {
namespace compact {

class CompactGenericRecordBuilderTest : public compact_test_base
{
protected:
    SerializationService& serialization_service()
    {
        return spi::ClientContext{ *client }.get_serialization_service();
    }
};

TEST_F(CompactGenericRecordBuilderTest, test_writing_same_field_multiple_times)
{
    using namespace serialization::generic_record;

    generic_record_builder builder{ random_string() };

    builder.set_string("name", "foo").set_int32("myint", 123);

    ASSERT_THROW(builder.set_string("name", "foo2"),
                 exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordBuilderTest,
       test_overwriting_same_field_multiple_times)
{
    using namespace serialization::generic_record;

    generic_record record = generic_record_builder{ random_string() }
                              .set_string("name", "foo")
                              .set_int32("myint", 123)
                              .build();

    generic_record_builder builder =
      record.new_builder_with_clone().set_string("name", "foo2");
    ASSERT_THROW(builder.set_string("name", "foo3"),
                 exception::hazelcast_serialization);
}

// It is modified version because there is no class definition
// for CompactGenericRecord, so record is filled according ClassDefinition
// and empty cloned.
TEST_F(CompactGenericRecordBuilderTest, test_writing_to_non_existing_field)
{
    using namespace serialization::generic_record;

    generic_record_builder builder = generic_record_builder{ random_string() }
                                       .set_string("name", "foo")
                                       .set_int32("myint", 123)
                                       .build()
                                       .new_builder();

    ASSERT_THROW(builder.set_string("nonExistingField", "foo3"),
                 exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordBuilderTest, test_writing_to_field_with_wrong_type)
{
    using namespace serialization::generic_record;

    generic_record_builder builder = generic_record_builder{ random_string() }
                                       .set_string("name", "foo")
                                       .set_int32("myint", 123)
                                       .build()
                                       .new_builder();

    ASSERT_THROW(builder.set_int32("name", 1),
                 exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordBuilderTest, test_unwritten_fields_throw_exception)
{
    using namespace serialization::generic_record;

    generic_record_builder builder = generic_record_builder{ random_string() }
                                       .set_string("name", "foo")
                                       .set_int32("myint", 123)
                                       .build()
                                       .new_builder();
    builder.set_int32("myint", 1);
    ASSERT_THROW(builder.build(), exception::hazelcast_serialization);
}

TEST_F(CompactGenericRecordBuilderTest,
       test_write_read_generic_record_to_object_data_input)
{
    using namespace serialization::generic_record;

    SerializationService& ss = serialization_service();
    std::string type_name = random_string();

    std::vector<generic_record> list;
    std::vector<serialization::pimpl::data> datas;

    for (int i = 0; i < 10; ++i) {
        generic_record record = generic_record_builder{ type_name }
                                  .set_int32("age", i)
                                  .set_string("name", " " + std::to_string(i))
                                  .build();

        list.push_back(record);
        datas.push_back(ss.to_data(record));
    }

    // Ensure that schema is distributed
    client->get_map(random_string())
      .get()
      ->put(random_string(), list.front())
      .get();

    for (int i = 0; i < 10; ++i) {
        auto& data = datas.at(i);
        auto& record = list.at(i);

        boost::optional<generic_record> actual =
          ss.to_object<generic_record>(data);

        EXPECT_TRUE(actual.has_value());
        EXPECT_EQ(record, *actual);
    }
}

TEST_F(CompactGenericRecordBuilderTest, test_try_to_write_after_record_is_built)
{
    using namespace serialization::generic_record;

    generic_record_builder normal_builder{ random_string() };

    auto record = normal_builder.set_int32("number", 1).build();

    generic_record_builder clone_builder = record.new_builder_with_clone();
    generic_record_builder bounded_builder = record.new_builder();

    bounded_builder.set_int32("number", 2);
    (void)bounded_builder.build();
    (void)clone_builder.build();

    EXPECT_THROW(normal_builder.set_string("another_field", "str"),
                 exception::hazelcast_serialization);
    EXPECT_THROW(bounded_builder.set_string("another_field", "str"),
                 exception::hazelcast_serialization);
    EXPECT_THROW(clone_builder.set_string("another_field", "str"),
                 exception::hazelcast_serialization);
}

} // namespace compact
} // namespace test
} // namespace client
} // namespace hazelcast