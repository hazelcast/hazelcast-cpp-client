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

#include <gtest/gtest.h>
#include "Serializables.h"

namespace hazelcast {
    namespace client {
        namespace test {
            employee::employee() = default;

            employee::employee(std::string name, int32_t age) : age_(age), name_(name) {
                by_ = 2;
                boolean_ = true;
                c_ = 'c';
                s_ = 4;
                i_ = 2000;
                l_ = 321324141;
                f_ = 3.14f;
                d_ = 3.14334;
                str_ = "Hello world";
                utf_str_ = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                byte byteArray[] = {50, 100, 150, 200};
                byte_vec_ = std::vector<byte>(byteArray, byteArray + 4);
                char charArray[] = {'c', 'h', 'a', 'r'};
                cc_ = std::vector<char>(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                ba_ = std::vector<bool>(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                ss_ = std::vector<int16_t>(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                ii_ = std::vector<int32_t>(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                ll_ = std::vector<int64_t>(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                ff_ = std::vector<float>(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                dd_ = std::vector<double>(doubleArray, doubleArray + 3);
            }

            bool employee::operator==(const employee &rhs) const {
                return age_ == rhs.get_age() && name_ == rhs.get_name();
            }

            bool employee::operator!=(const employee &employee) const {
                return !(*this == employee);
            }

            int32_t employee::get_age() const {
                return age_;
            }

            const std::string &employee::get_name() const {
                return name_;
            }

            bool employee::operator<(const employee &rhs) const {
                return age_ < rhs.get_age();
            }

            int32_t EmployeeEntryComparator::compare(const std::pair<const int32_t *, const employee *> *lhs,
                                                     const std::pair<const int32_t *, const employee *> *rhs) const {
                const employee *lv = lhs->second;
                const employee *rv = rhs->second;

                if (nullptr == lv && nullptr == rv) {
                    // order by key
                    const int32_t leftKey = *lhs->first;
                    const int32_t rightKey = *rhs->first;

                    if (leftKey == rightKey) {
                        return 0;
                    }

                    if (leftKey < rightKey) {
                        return -1;
                    }

                    return 1;
                }

                if (nullptr == lv) {
                    return -1;
                }

                if (nullptr == rv) {
                    return 1;
                }

                int32_t la = lv->get_age();
                int32_t ra = rv->get_age();

                if (la == ra) {
                    return 0;
                }

                if (la < ra) {
                    return -1;
                }

                return 1;
            }


            int32_t EmployeeEntryKeyComparator::compare(const std::pair<const int32_t *, const employee *> *lhs,
                                                        const std::pair<const int32_t *, const employee *> *rhs) const {
                const int32_t *key1 = lhs->first;
                const int32_t *key2 = rhs->first;

                if (nullptr == key1) {
                    return -1;
                }

                if (nullptr == key2) {
                    return 1;
                }

                if (*key1 == *key2) {
                    return 0;
                }

                if (*key1 < *key2) {
                    return -1;
                }

                return 1;
            }

            std::ostream &operator<<(std::ostream &out, const employee &employee) {
                out << "Employee:[" << employee.get_name() << ", " << employee.get_age() << "]";
                return out;
            }

            bool operator==(const TestCustomXSerializable &lhs, const TestCustomXSerializable &rhs) {
                return lhs.id == rhs.id;
            }

            bool operator==(const TestCustomPerson &lhs, const TestCustomPerson &rhs) {
                return lhs.name == rhs.name;
            }

            bool operator==(const TestRawDataPortable &lhs, const TestRawDataPortable &rhs) {
                return lhs.l == rhs.l &&
                       lhs.c == rhs.c &&
                       lhs.p == rhs.p &&
                       lhs.k == rhs.k &&
                       lhs.s == rhs.s &&
                       lhs.ds == rhs.ds;
            }

            bool operator==(const TestDataSerializable &lhs, const TestDataSerializable &rhs) {
                return lhs.i == rhs.i &&
                       lhs.c == rhs.c;
            }

            bool operator==(const TestNamedPortableV2 &lhs, const TestNamedPortableV2 &rhs) {
                return lhs.name == rhs.name &&
                       lhs.k == rhs.k &&
                       lhs.v == rhs.v;
            }

            bool operator==(const TestInnerPortable &lhs, const TestInnerPortable &rhs) {
                return lhs.bb == rhs.bb &&
                       lhs.ba == rhs.ba &&
                       lhs.cc == rhs.cc &&
                       lhs.ss == rhs.ss &&
                       lhs.ii == rhs.ii &&
                       lhs.ll == rhs.ll &&
                       lhs.ff == rhs.ff &&
                       lhs.dd == rhs.dd &&
                       lhs.string_vector == rhs.string_vector &&
                       lhs.nn == rhs.nn;
            }

            bool operator==(const TestNamedPortable &lhs, const TestNamedPortable &rhs) {
                return lhs.name == rhs.name &&
                       lhs.k == rhs.k;
            }

            bool operator==(const TestMainPortable &lhs, const TestMainPortable &rhs) {
                return lhs.b == rhs.b &&
                       lhs.boolean == rhs.boolean &&
                       lhs.c == rhs.c &&
                       lhs.s == rhs.s &&
                       lhs.i == rhs.i &&
                       lhs.l == rhs.l &&
                       lhs.f == rhs.f &&
                       lhs.d == rhs.d &&
                       lhs.str == rhs.str &&
                       lhs.p == rhs.p;
            }

            bool operator==(const TestNamedPortableV3 &lhs, const TestNamedPortableV3 &rhs) {
                return lhs.name == rhs.name &&
                       lhs.k == rhs.k;
            }

            bool operator==(const ChildTemplatedPortable1 &lhs, const ChildTemplatedPortable1 &rhs) {
                return lhs.s1 == rhs.s1 &&
                       lhs.s2 == rhs.s2;
            }

            bool operator==(const ChildTemplatedPortable2 &lhs, const ChildTemplatedPortable2 &rhs) {
                return lhs.s1 == rhs.s1;
            }
        }

        namespace serialization {
            int32_t hz_serializer<test::employee>::get_factory_id() {
                return 666;
            }

            int32_t hz_serializer<test::employee>::get_class_id() {
                return 2;
            }

            void hz_serializer<test::employee>::write_portable(const test::employee &object, portable_writer &writer) {
                writer.write("n", object.name_);
                writer.write<int32_t>("a", object.age_);

                writer.write<byte>("b", object.by_);
                writer.write("c", object.c_);
                writer.write("bo", object.boolean_);
                writer.write<int16_t>("s", object.s_);
                writer.write<int32_t>("i", object.i_);
                writer.write<int64_t>("l", object.l_);
                writer.write<float>("f", object.f_);
                writer.write<double>("d", object.d_);
                writer.write("str", object.str_);
                writer.write("utfstr", &object.utf_str_);

                writer.write("bb", object.byte_vec_);
                writer.write("cc", object.cc_);
                writer.write("ba", object.ba_);
                writer.write("ss", object.ss_);
                writer.write("ii", object.ii_);
                writer.write("ff", object.ff_);
                writer.write("dd", object.dd_);

                serialization::object_data_output &out = writer.get_raw_data_output();
                out.write_object<byte>(&object.by_);
                out.write_object<char>(object.c_);
                out.write_object<bool>(&object.boolean_);
                out.write_object<int16_t>(&object.s_);
                out.write_object<int32_t>(&object.i_);
                out.write_object<float>(&object.f_);
                out.write_object<double>(&object.d_);
                out.write_object<std::string>(&object.str_);
                out.write_object<std::string>(&object.utf_str_);
            }

            test::employee hz_serializer<test::employee>::read_portable(portable_reader &reader) {
                test::employee employee;
                employee.name_ = reader.read<std::string>("n");
                employee.age_ = reader.read<int32_t>("a");
                employee.by_ = reader.read<byte>("b");
                employee.c_ = reader.read<char>("c");
                employee.boolean_ = reader.read<bool>("bo");
                employee.s_ = reader.read<int16_t>("s");
                employee.i_ = reader.read<int32_t>("i");
                employee.l_ = reader.read<int64_t>("l");
                employee.f_ = reader.read<float>("f");
                employee.d_ = reader.read<double>("d");
                employee.str_ = reader.read<std::string>("str");
                employee.utf_str_ = reader.read<std::string>("utfstr");
                employee.byte_vec_ = *reader.read<std::vector<byte>>("bb");
                employee.cc_ = *reader.read<std::vector<char>>("cc");
                employee.ba_ = *reader.read<std::vector<bool>>("ba");
                employee.ss_ = *reader.read<std::vector<int16_t>>("ss");
                employee.ii_ = *reader.read<std::vector<int32_t>>("ii");
                employee.ff_ = *reader.read<std::vector<float>>("ff");
                employee.dd_ = *reader.read<std::vector<double>>("dd");
                serialization::object_data_input &in = reader.get_raw_data_input();
                employee.by_ = *in.read_object<byte>();
                employee.c_ = *in.read_object<char>();
                employee.boolean_ = *in.read_object<bool>();
                employee.s_ = *in.read_object<int16_t>();
                employee.i_ = *in.read_object<int32_t>();
                employee.f_ = *in.read_object<float>();
                employee.d_ = *in.read_object<double>();
                employee.str_ = *in.read_object<std::string>();
                employee.utf_str_ = *in.read_object<std::string>();
                return employee;
            }

            int32_t hz_serializer<test::EmployeeEntryComparator>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::EMPLOYEE_FACTORY);
            }

            int32_t hz_serializer<test::EmployeeEntryComparator>::get_class_id() {
                return 4;
            }

            void hz_serializer<test::EmployeeEntryComparator>::write_data(const test::EmployeeEntryComparator &object,
                                                                          object_data_output &writer) {}

            test::EmployeeEntryComparator
            hz_serializer<test::EmployeeEntryComparator>::read_data(object_data_input &reader) {
                return test::EmployeeEntryComparator();
            }

            int32_t hz_serializer<test::EmployeeEntryKeyComparator>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::EMPLOYEE_FACTORY);
            }

            int32_t hz_serializer<test::EmployeeEntryKeyComparator>::get_class_id() {
                return 5;
            }

            void hz_serializer<test::EmployeeEntryKeyComparator>::write_data(const test::EmployeeEntryKeyComparator &,
                                                                             object_data_output &) {}

            test::EmployeeEntryKeyComparator hz_serializer<test::EmployeeEntryKeyComparator>::read_data(object_data_input &) {
                return test::EmployeeEntryKeyComparator();
            }

            int32_t hz_serializer<test::TestMainPortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestMainPortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_MAIN_PORTABLE);
            }

            void hz_serializer<test::TestMainPortable>::write_portable(const test::TestMainPortable &object,
                                                                      serialization::portable_writer &writer) {
                writer.write<byte>("b", object.b);
                writer.write("bool", object.boolean);
                writer.write("c", object.c);
                writer.write<int16_t>("s", object.s);
                writer.write<int32_t>("i", object.i);
                writer.write<int64_t>("l", object.l);
                writer.write<float>("f", object.f);
                writer.write<double>("d", object.d);
                writer.write("str", object.str);
                writer.write_portable("p", &object.p);
            }

            test::TestMainPortable
            hz_serializer<test::TestMainPortable>::read_portable(serialization::portable_reader &reader) {
                test::TestMainPortable object;
                object.b = reader.read<byte>("b");
                object.boolean = reader.read<bool>("bool");
                object.c = reader.read<char>("c");
                object.s = reader.read<int16_t>("s");
                object.i = reader.read<int32_t>("i");
                object.l = reader.read<int64_t>("l");
                object.f = reader.read<float>("f");
                object.d = reader.read<double>("d");
                object.str = reader.read<std::string>("str");
                auto innerPortable = reader.read_portable<test::TestInnerPortable>("p");
                if (innerPortable.has_value())
                    object.p = std::move(innerPortable).value();

                return object;
            }

            int32_t hz_serializer<test::TestRawDataPortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestRawDataPortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_RAW_DATA_PORTABLE);
            }

            void hz_serializer<test::TestRawDataPortable>::write_portable(const test::TestRawDataPortable &object,
                                                                         serialization::portable_writer &writer) {
                writer.write("l", object.l);
                writer.write("c", object.c);
                writer.write_portable("p", &object.p);
                serialization::object_data_output &out = writer.get_raw_data_output();
                out.write<int32_t>(object.k);
                out.write(object.s);
                hz_serializer<test::TestDataSerializable>::write_data(object.ds, out);
            }

            test::TestRawDataPortable
            hz_serializer<test::TestRawDataPortable>::read_portable(serialization::portable_reader &reader) {
                test::TestRawDataPortable object;
                object.l = reader.read<int64_t>("l");
                object.c = std::move(reader.read<std::vector<char>>("c")).value();
                auto namedPortable = reader.read_portable<test::TestNamedPortable>("p");
                if (namedPortable.has_value())
                    object.p = namedPortable.value();
                serialization::object_data_input &in = reader.get_raw_data_input();
                object.k = in.read<int32_t>();
                object.s = in.read<std::string>();
                object.ds = hz_serializer<test::TestDataSerializable>::read_data(in);
                return object;
            }

            int32_t hz_serializer<test::TestDataSerializable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_DATA_FACTORY);
            }

            int32_t hz_serializer<test::TestDataSerializable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_DATA_SERIALIZABLE);
            }

            void
            hz_serializer<test::TestDataSerializable>::write_data(const test::TestDataSerializable &object,
                                                                 serialization::object_data_output &out) {
                out.write<int32_t>(object.i);
                out.write(object.c);
            }

            test::TestDataSerializable
            hz_serializer<test::TestDataSerializable>::read_data(serialization::object_data_input &in) {
                return test::TestDataSerializable{in.read<int32_t>(), in.read<char>()};
            }

            int32_t hz_serializer<test::TestNamedPortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_NAMED_PORTABLE);
            }

            void hz_serializer<test::TestNamedPortable>::write_portable(const test::TestNamedPortable &object,
                                                                        portable_writer &writer) {
                writer.write("name", object.name);
                writer.write<int32_t>("myint", object.k);
            }

            test::TestNamedPortable hz_serializer<test::TestNamedPortable>::read_portable(portable_reader &reader) {
                return test::TestNamedPortable{reader.read<std::string>("name"), reader.read<int32_t>("myint")};
            }

            int32_t hz_serializer<test::TestInnerPortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInnerPortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_INNER_PORTABLE);
            }

            void hz_serializer<test::TestInnerPortable>::write_portable(const test::TestInnerPortable &object,
                                                                        portable_writer &writer) {
                writer.write("b", object.bb);
                writer.write("ba", object.ba);
                writer.write("c", object.cc);
                writer.write("s", object.ss);
                writer.write("i", object.ii);
                writer.write("l", object.ll);
                writer.write("f", object.ff);
                writer.write("d", object.dd);
                writer.write("stringVector", object.string_vector);
                writer.write_portable_array("nn", &object.nn);
            }

            test::TestInnerPortable hz_serializer<test::TestInnerPortable>::read_portable(portable_reader &reader) {
                test::TestInnerPortable object;
                object.bb = *reader.read<std::vector<byte>>("b");
                object.ba = *reader.read<std::vector<bool>>("ba");
                object.cc = *reader.read<std::vector<char>>("c");
                object.ss = *reader.read<std::vector<int16_t>>("s");
                object.ii = *reader.read<std::vector<int32_t>>("i");
                object.ll = *reader.read<std::vector<int64_t>>("l");
                object.ff = *reader.read<std::vector<float>>("f");
                object.dd = *reader.read<std::vector<double>>("d");
                object.string_vector = *reader.read<std::vector<std::string>>("stringVector");
                object.nn = *reader.read_portable_array<test::TestNamedPortable>("nn");
                return object;
            }

            int32_t hz_serializer<test::TestNamedPortableV2>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortableV2>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_NAMED_PORTABLE);
            }

            int32_t hz_serializer<test::TestNamedPortableV2>::get_class_version() {
                return 2;
            }

            void hz_serializer<test::TestNamedPortableV2>::write_portable(const test::TestNamedPortableV2 &object,
                                                                          portable_writer &writer) {
                writer.write("name", object.name);
                writer.write<int32_t>("myint", object.k);
                writer.write<int32_t>("v", object.v);
            }

            test::TestNamedPortableV2 hz_serializer<test::TestNamedPortableV2>::read_portable(portable_reader &reader) {
                return test::TestNamedPortableV2{reader.read<std::string>("name"), reader.read<int32_t>("myint"),
                                                 reader.read<int32_t>("v")};
            }

            int32_t hz_serializer<test::TestNamedPortableV3>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortableV3>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_NAMED_PORTABLE);
            }

            int32_t hz_serializer<test::TestNamedPortableV3>::get_class_version() {
                return 3;
            }

            void hz_serializer<test::TestNamedPortableV3>::write_portable(const test::TestNamedPortableV3 &object,
                                                                          portable_writer &writer) {
                writer.write("name", object.name);
                writer.write<int16_t>("myint", object.k);
            }

            test::TestNamedPortableV3 hz_serializer<test::TestNamedPortableV3>::read_portable(portable_reader &reader) {
                return test::TestNamedPortableV3{reader.read<std::string>("name"), reader.read<int16_t>("myint")};
            }

            int32_t hz_serializer<test::TestInvalidWritePortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInvalidWritePortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_INVALID_WRITE_PORTABLE);
            }

            void
            hz_serializer<test::TestInvalidWritePortable>::write_portable(const test::TestInvalidWritePortable &object,
                                                                          portable_writer &writer) {
                writer.write<int64_t>("l", object.l);
                serialization::object_data_output &out = writer.get_raw_data_output();
                out.write<int32_t>(object.i);
                writer.write("s", object.s);
            }

            test::TestInvalidWritePortable
            hz_serializer<test::TestInvalidWritePortable>::read_portable(portable_reader &reader) {
                return test::TestInvalidWritePortable{reader.read<int64_t>("l"), reader.read<int32_t>("i"),
                                                      reader.read<std::string>("s")};
            }

            int32_t hz_serializer<test::TestInvalidReadPortable>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInvalidReadPortable>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_INVALID_READ_PORTABLE);
            }

            void
            hz_serializer<test::TestInvalidReadPortable>::write_portable(const test::TestInvalidReadPortable &object,
                                                                         portable_writer &writer) {
                writer.write<int64_t>("l", object.l);
                writer.write<int32_t>("i", object.i);
                writer.write("s", object.s);
            }

            test::TestInvalidReadPortable
            hz_serializer<test::TestInvalidReadPortable>::read_portable(portable_reader &reader) {
                test::TestInvalidReadPortable obj;
                obj.l = reader.read<int64_t>("l");
                serialization::object_data_input &in = reader.get_raw_data_input();
                obj.i = in.read<int32_t>();
                obj.s = reader.read<std::string>("s");
                return obj;
            }

            void hz_serializer<test::TestCustomPerson>::write(const test::TestCustomPerson &object, object_data_output &out) {
                out.write<int32_t>(999);
                out.write(object.name);
                out.write<int32_t>(999);
            }

            test::TestCustomPerson hz_serializer<test::TestCustomPerson>::read(object_data_input &in) {
                {
                    int t = in.read<int32_t>();
                    assert(999 == t);
                }

                test::TestCustomPerson object{in.read<std::string>()};
                
                {
                    int t = in.read<int32_t>();
                    assert(999 == t);
                }

                return object;
            }

            void hz_serializer<test::TestCustomXSerializable>::write(const test::TestCustomXSerializable &object, object_data_output &out) {
                out.write<int32_t>(666);
                out.write(object.id);
                out.write<int32_t>(666);
            }

            test::TestCustomXSerializable hz_serializer<test::TestCustomXSerializable>::read(object_data_input &in) {
                {
                    int t = in.read<int32_t>();
                    assert(666 == t);
                }

                test::TestCustomXSerializable object{in.read<int32_t>()};
                
                {
                    int t = in.read<int32_t>();
                    assert(666 == t);
                }
                
                return object;
            }

            int32_t hz_serializer<test::ChildTemplatedPortable1>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::ChildTemplatedPortable1>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::CHILD_TEMPLATED_PORTABLE_1);
            }

            void
            hz_serializer<test::ChildTemplatedPortable1>::write_portable(const test::ChildTemplatedPortable1 &object,
                                                                         portable_writer &out) {
                out.write("s1", object.s1);
                out.write("s1", object.s2);
            }

            test::ChildTemplatedPortable1
            hz_serializer<test::ChildTemplatedPortable1>::read_portable(portable_reader &reader) {
                return test::ChildTemplatedPortable1{reader.read<std::string>("s1"), reader.read<std::string>("s2")};
            }

            int32_t hz_serializer<test::ChildTemplatedPortable2>::get_factory_id() {
                return static_cast<int32_t>(test::test_serialization_constants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::ChildTemplatedPortable2>::get_class_id() {
                return static_cast<int32_t>(test::test_serialization_constants::CHILD_TEMPLATED_PORTABLE_2);
            }

            void
            hz_serializer<test::ChildTemplatedPortable2>::write_portable(const test::ChildTemplatedPortable2 &object,
                                                                         portable_writer &out) {
                out.write("s1", object.s1);
            }

            test::ChildTemplatedPortable2
            hz_serializer<test::ChildTemplatedPortable2>::read_portable(portable_reader &reader) {
                return test::ChildTemplatedPortable2{reader.read<std::string>("s1")};
            }

        }
    }
}

