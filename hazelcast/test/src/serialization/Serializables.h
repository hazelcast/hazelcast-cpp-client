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

#include <hazelcast/client/query/EntryComparator.h>

namespace hazelcast {
    namespace client {
        namespace test {
            enum struct TestSerializationConstants {
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

            class Employee {
                friend serialization::hz_serializer<Employee>;
            public:
                Employee();

                Employee(std::string name, int age);

                bool operator ==(const Employee &employee) const;

                bool operator !=(const Employee &employee) const;

                int32_t get_age() const;

                const std::string &get_name() const;

                bool operator<(const Employee &rhs) const;

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
                std::string utfStr_;

                std::vector<byte> byteVec_;
                std::vector<char> cc_;
                std::vector<bool> ba_;
                std::vector<int16_t> ss_;
                std::vector<int32_t> ii_;
                std::vector<int64_t > ll_;
                std::vector<float> ff_;
                std::vector<double> dd_;
            };

            // Compares based on the employee age
            class EmployeeEntryComparator : public query::EntryComparator<int32_t, Employee> {
            public:
                int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                                    const std::pair<const int32_t *, const Employee *> *rhs) const override;
            };

            // Compares based on the employee age
            class EmployeeEntryKeyComparator : public EmployeeEntryComparator {
            public:
                int compare(const std::pair<const int32_t *, const Employee *> *lhs,
                            const std::pair<const int32_t *, const Employee *> *rhs) const override;
            };

            std::ostream &operator<<(std::ostream &out, const Employee &employee);

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
            struct hz_serializer<test::Employee> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::Employee &object, PortableWriter &writer);

                static test::Employee read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::EmployeeEntryComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::EmployeeEntryComparator &object, ObjectDataOutput &writer);

                static test::EmployeeEntryComparator read_data(ObjectDataInput &reader);
            };

            template<>
            struct hz_serializer<test::EmployeeEntryKeyComparator> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::EmployeeEntryKeyComparator &object, ObjectDataOutput &writer);

                static test::EmployeeEntryKeyComparator read_data(ObjectDataInput &reader);
            };

            template<>
            struct hz_serializer<test::TestMainPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestMainPortable &object, PortableWriter &writer);

                static test::TestMainPortable read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestRawDataPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestRawDataPortable &object, PortableWriter &writer);

                static test::TestRawDataPortable read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestDataSerializable> : public identified_data_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_data(const test::TestDataSerializable &object, ObjectDataOutput &out);

                static test::TestDataSerializable read_data(ObjectDataInput &in);
            };

            template<>
            struct hz_serializer<test::TestInnerPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInnerPortable &object, PortableWriter &writer);

                static test::TestInnerPortable read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestNamedPortable &object, PortableWriter &writer);

                static test::TestNamedPortable read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortableV2> : public versioned_portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static int32_t get_class_version();

                static void write_portable(const test::TestNamedPortableV2 &object, PortableWriter &writer);

                static test::TestNamedPortableV2 read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestNamedPortableV3> : public versioned_portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static int32_t get_class_version();

                static void write_portable(const test::TestNamedPortableV3 &object, PortableWriter &writer);

                static test::TestNamedPortableV3 read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestInvalidWritePortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInvalidWritePortable &object, PortableWriter &writer);

                static test::TestInvalidWritePortable read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::TestInvalidReadPortable> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::TestInvalidReadPortable &object, PortableWriter &writer);

                static test::TestInvalidReadPortable read_portable(PortableReader &reader);
            };

            template<typename P>
            struct hz_serializer<test::ObjectCarryingPortable<P>> : public portable_serializer {
                static int32_t get_factory_id() {
                    return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
                }

                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::TestSerializationConstants::OBJECT_CARRYING_PORTABLE);
                }

                static void write_portable(const test::ObjectCarryingPortable<P> &object, PortableWriter &writer) {
                    auto &output = writer.get_raw_data_output();
                    output.write_object<P>(object.carried_object);
                }

                static test::ObjectCarryingPortable<P> read_portable(PortableReader &reader) {
                    ObjectDataInput& input = reader.get_raw_data_input();
                    return test::ObjectCarryingPortable<P>{input.read_object<P>().value()};
                }
            };

            template<>
            struct hz_serializer<test::ChildTemplatedPortable1> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::ChildTemplatedPortable1 &object, PortableWriter &writer);

                static test::ChildTemplatedPortable1 read_portable(PortableReader &reader);
            };

            template<>
            struct hz_serializer<test::ChildTemplatedPortable2> : public portable_serializer {
                static int32_t get_factory_id();

                static int32_t get_class_id();

                static void write_portable(const test::ChildTemplatedPortable2 &object, PortableWriter &writer);

                static test::ChildTemplatedPortable2 read_portable(PortableReader &reader);
            };

            template<typename P>
            struct hz_serializer<test::ParentTemplatedPortable<P>> : public portable_serializer {
                static int32_t get_factory_id() {
                    return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
                }

                static int32_t get_class_id() {
                    return static_cast<int32_t>(test::TestSerializationConstants::PARENT_TEMPLATED_CONSTANTS);
                }

                static void write_portable(const test::ParentTemplatedPortable<P> &object, PortableWriter &out) {
                    out.write_portable("c", object.child.get_ptr());
                }

                static test::ParentTemplatedPortable<P> read_portable(PortableReader &in) {
                    return test::ParentTemplatedPortable<P>{in.read_portable<P>()};
                }
            };

            template<>
            struct hz_serializer<test::TestCustomPerson> : public custom_serializer {
                static constexpr int32_t get_type_id() {
                    return 666;
                }

                static void write(const test::TestCustomPerson &object, ObjectDataOutput & out);

                static test::TestCustomPerson read(ObjectDataInput &in);
            };

            template<>
            struct hz_serializer<test::TestCustomXSerializable> : public custom_serializer {
                static constexpr int32_t get_type_id() {
                    return 666;
                }

                static void write(const test::TestCustomXSerializable &object, ObjectDataOutput &out);

                static test::TestCustomXSerializable read(ObjectDataInput &in);
            };

        }
    }
}

