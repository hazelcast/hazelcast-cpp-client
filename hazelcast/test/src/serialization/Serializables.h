/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <cstdint>
#include <ostream>
#include <vector>
#include <string>

#include <hazelcast/client/query/entry_comparator.h>

namespace hazelcast {
    namespace client {
        namespace test {
            enum struct test_serialization_constants {
                EMPLOYEE = 2,
                CHILD_TEMPLATED_PORTABLE_1 = 1,
                CHILD_TEMPLATED_PORTABLE_2 = 2,
                OBJECT_CARRYING_PORTABLE = 4,
                PARENT_TEMPLATED_CONSTANTS = 5,
                TEST_DATA_SERIALIZABLE = 6,
                TEST_INNER_PORTABLE = 7,
                TEST_INVALID_READ_PORTABLE = 8,
                TEST_INVALID_WRITE_PORTABLE = 9,
                TEST_MAIN_PORTABLE = 10,
                TEST_NAMED_PORTABLE = 11,
                TEST_NAMED_PORTABLE_2 = 12,
                TEST_NAMED_PORTABLE_3 = 13,
                TEST_RAW_DATA_PORTABLE = 14,

                TEST_PORTABLE_FACTORY = 1,
                TEST_DATA_FACTORY = 1,
                EMPLOYEE_FACTORY = 666
            };

            class employee {
                friend serialization::hz_serializer<employee>;
            public:
                employee();

                employee(std::string name, int age);

                bool operator ==(const employee &employee) const;

                bool operator !=(const employee &employee) const;

                int32_t get_age() const;

                const std::string &get_name() const;

                bool operator<(const employee &rhs) const;

            private:
                int32_t age_;
                std::string name_;

                // add all possible types
                byte by_;
                bool boolean_;
                char c_;
                int16_t s_;
                int32_t i_;
                int64_t  l_;
                float f_;
                double d_;
                std::string str_;
                std::string utf_str_;

                std::vector<byte> byte_vec_;
                std::vector<char> cc_;
                std::vector<bool> ba_;
                std::vector<int16_t> ss_;
                std::vector<int32_t> ii_;
                std::vector<int64_t > ll_;
                std::vector<float> ff_;
                std::vector<double> dd_;
            };

            // Compares based on the employee age
            class EmployeeEntryComparator : public query::entry_comparator<int32_t, employee> {
            public:
                int compare(const std::pair<const int32_t *, const employee *> *lhs,
                                    const std::pair<const int32_t *, const employee *> *rhs) const override;
            };

            // Compares based on the employee age
            class EmployeeEntryKeyComparator : public EmployeeEntryComparator {
            public:
                int compare(const std::pair<const int32_t *, const employee *> *lhs,
                            const std::pair<const int32_t *, const employee *> *rhs) const override;
            };

            std::ostream &operator<<(std::ostream &out, const employee &employee);

            struct TestDataSerializable {
                int i;

                friend bool operator==(const TestDataSerializable &lhs, const TestDataSerializable &rhs);

                char c;
            };

            struct TestNamedPortable {
                std::string name;
                int k;

                friend bool operator==(const TestNamedPortable &lhs, const TestNamedPortable &rhs);
            };

            struct TestRawDataPortable {
                int64_t l;
                std::vector<char> c;
                TestNamedPortable p;
                int32_t k;
                std::string s;
                TestDataSerializable ds;

                friend bool operator==(const TestRawDataPortable &lhs, const TestRawDataPortable &rhs);
            };

            struct TestInnerPortable {
                std::vector<byte> bb;
                std::vector<bool> ba;
                std::vector<char> cc;
                std::vector<int16_t> ss;
                std::vector<int32_t> ii;
                std::vector<int64_t> ll;
                std::vector<float> ff;
                std::vector<double> dd;
                std::vector<std::string> string_vector;
                std::vector<TestNamedPortable> nn;

                friend bool operator==(const TestInnerPortable &lhs, const TestInnerPortable &rhs);
            };

            struct TestMainPortable {
                byte b;
                bool boolean;
                char c;
                short s;
                int i;
                int64_t l;
                float f;
                double d;
                std::string str;
                TestInnerPortable p;

                friend bool operator==(const TestMainPortable &lhs, const TestMainPortable &rhs);
            };

            struct TestInvalidReadPortable {
                int64_t l;
                int32_t i;
                std::string s;
            };

            struct TestInvalidWritePortable {
                int64_t l;
                int32_t i;
                std::string s;
            };

            struct TestNamedPortableV2 {
                std::string name;
                int k;
                int v;

                friend bool operator==(const TestNamedPortableV2 &lhs, const TestNamedPortableV2 &rhs);
            };

            struct TestNamedPortableV3 {
                std::string name;
                short k;

                friend bool operator==(const TestNamedPortableV3 &lhs, const TestNamedPortableV3 &rhs);
            };

            struct TestCustomXSerializable {
                int32_t id;

                friend bool operator==(const TestCustomXSerializable &lhs, const TestCustomXSerializable &rhs);
            };

            struct TestCustomPerson {
                std::string name;

                friend bool operator==(const TestCustomPerson &lhs, const TestCustomPerson &rhs);
            };

            template<typename P>
            struct ObjectCarryingPortable {
                P carried_object;

                friend bool operator==(const ObjectCarryingPortable &lhs, const ObjectCarryingPortable &rhs) {
                    return lhs.carried_object == rhs.carried_object;
                }
            };

            template<typename T>
            struct ParentTemplatedPortable {
                boost::optional<T> child;

                friend bool operator==(const ParentTemplatedPortable &lhs, const ParentTemplatedPortable &rhs) {
                    return lhs.child == rhs.child;
                }
            };

            struct ChildTemplatedPortable1 {
                std::string s1;
                std::string s2;

                friend bool operator==(const ChildTemplatedPortable1 &lhs, const ChildTemplatedPortable1 &rhs);
            };

            struct ChildTemplatedPortable2 {
                std::string s1;

                friend bool operator==(const ChildTemplatedPortable2 &lhs, const ChildTemplatedPortable2 &rhs);
            };
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::employee> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::employee &object, portable_writer &writer);

                static test::employee read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::EmployeeEntryComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::EmployeeEntryComparator &object, object_data_output &writer);

                static test::EmployeeEntryComparator read_data(object_data_input &reader);
            };

            template<>
            struct hz_serializer<test::EmployeeEntryKeyComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::EmployeeEntryKeyComparator &object, object_data_output &writer);

                static test::EmployeeEntryKeyComparator read_data(object_data_input &reader);
            };

            template<>
            struct hz_serializer<test::TestMainPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestMainPortable &object, portable_writer &writer);

                static test::TestMainPortable read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestRawDataPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestRawDataPortable &object, portable_writer &writer);

                static test::TestRawDataPortable read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestDataSerializable> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::TestDataSerializable &object, object_data_output &out);

                static test::TestDataSerializable read_data(object_data_input &in);
            };

            template<>
            struct hz_serializer<test::TestInnerPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInnerPortable &object, portable_writer &writer);

                static test::TestInnerPortable read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestNamedPortable &object, portable_writer &writer);

                static test::TestNamedPortable read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortableV2> : public versioned_portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static int32_t get_class_version();

                static void write_portable(const test::TestNamedPortableV2 &object, portable_writer &writer);

                static test::TestNamedPortableV2 read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortableV3> : public versioned_portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static int32_t get_class_version();

                static void write_portable(const test::TestNamedPortableV3 &object, portable_writer &writer);

                static test::TestNamedPortableV3 read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestInvalidWritePortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInvalidWritePortable &object, portable_writer &writer);

                static test::TestInvalidWritePortable read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::TestInvalidReadPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInvalidReadPortable &object, portable_writer &writer);

                static test::TestInvalidReadPortable read_portable(portable_reader &reader);
            };

            template<typename P>
            struct hz_serializer<test::ObjectCarryingPortable<P>> : public portable_serializer {
                static int32_t get_factory_id() {
                    return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
                }

                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::test_serialization_constants::OBJECT_CARRYING_PORTABLE);
                }

                static void write_portable(const test::ObjectCarryingPortable<P> &object, portable_writer &writer) {
                    auto &output = writer.get_raw_data_output();
                    output.write_object<P>(object.carried_object);
                }

                static test::ObjectCarryingPortable<P> read_portable(portable_reader &reader) {
                    object_data_input& input = reader.get_raw_data_input();
                    return test::ObjectCarryingPortable<P>{input.read_object<P>().value()};
                }
            };

            template<>
            struct hz_serializer<test::ChildTemplatedPortable1> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::ChildTemplatedPortable1 &object, portable_writer &writer);

                static test::ChildTemplatedPortable1 read_portable(portable_reader &reader);
            };

            template<>
            struct hz_serializer<test::ChildTemplatedPortable2> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::ChildTemplatedPortable2 &object, portable_writer &writer);

                static test::ChildTemplatedPortable2 read_portable(portable_reader &reader);
            };

            template<typename P>
            struct hz_serializer<test::ParentTemplatedPortable<P>> : public portable_serializer {
                static int32_t get_factory_id() {
                    return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
                }

                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::test_serialization_constants::PARENT_TEMPLATED_CONSTANTS);
                }

                static void write_portable(const test::ParentTemplatedPortable<P> &object, portable_writer &out) {
                    out.write_portable("c", object.child.get_ptr());
                }

                static test::ParentTemplatedPortable<P> read_portable(portable_reader &in) {
                    return test::ParentTemplatedPortable<P>{in.read_portable<P>()};
                }
            };

            template<>
            struct hz_serializer<test::TestCustomPerson> : public custom_serializer {
                static constexpr int32_t get_type_id() {
                    return 666;
                }

                static void write(const test::TestCustomPerson &object, object_data_output & out);

                static test::TestCustomPerson read(object_data_input &in);
            };

            template<>
            struct hz_serializer<test::TestCustomXSerializable> : public custom_serializer {
                static constexpr int32_t get_type_id() {
                    return 666;
                }

                static void write(const test::TestCustomXSerializable &object, object_data_output &out);

                static test::TestCustomXSerializable read(object_data_input &in);
            };

        }
    }
}

