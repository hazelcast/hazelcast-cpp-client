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
            Employee::Employee() {}

            Employee::Employee(std::string name, int32_t age) : age(age), name(name) {
                by = 2;
                boolean = true;
                c = 'c';
                s = 4;
                i = 2000;
                l = 321324141;
                f = 3.14f;
                d = 3.14334;
                str = "Hello world";
                utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                byte byteArray[] = {50, 100, 150, 200};
                byteVec = std::vector<byte>(byteArray, byteArray + 4);
                char charArray[] = {'c', 'h', 'a', 'r'};
                cc = std::vector<char>(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                ba = std::vector<bool>(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                ss = std::vector<int16_t>(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                ii = std::vector<int32_t>(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                ll = std::vector<int64_t>(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                ff = std::vector<float>(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                dd = std::vector<double>(doubleArray, doubleArray + 3);
            }

            bool Employee::operator==(const Employee &rhs) const {
                return age == rhs.getAge() && name == rhs.getName();
            }

            bool Employee::operator!=(const Employee &employee) const {
                return !(*this == employee);
            }

            int32_t Employee::getAge() const {
                return age;
            }

            const std::string &Employee::getName() const {
                return name;
            }

            bool Employee::operator<(const Employee &rhs) const {
                return age < rhs.getAge();
            }

            int32_t EmployeeEntryComparator::compare(const std::pair<const int32_t *, const Employee *> *lhs,
                                                     const std::pair<const int32_t *, const Employee *> *rhs) const {
                const Employee *lv = lhs->second;
                const Employee *rv = rhs->second;

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

                int32_t la = lv->getAge();
                int32_t ra = rv->getAge();

                if (la == ra) {
                    return 0;
                }

                if (la < ra) {
                    return -1;
                }

                return 1;
            }


            int32_t EmployeeEntryKeyComparator::compare(const std::pair<const int32_t *, const Employee *> *lhs,
                                                        const std::pair<const int32_t *, const Employee *> *rhs) const {
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

            std::ostream &operator<<(std::ostream &out, const Employee &employee) {
                out << "Employee:[" << employee.getName() << ", " << employee.getAge() << "]";
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
            int32_t hz_serializer<test::Employee>::getFactoryId() {
                return 666;
            }

            int32_t hz_serializer<test::Employee>::getClassId() {
                return 2;
            }

            void hz_serializer<test::Employee>::writePortable(const test::Employee &object, PortableWriter &writer) {
                writer.write("n", object.name);
                writer.write<int32_t>("a", object.age);

                writer.write<byte>("b", object.by);
                writer.write("c", object.c);
                writer.write("bo", object.boolean);
                writer.write<int16_t>("s", object.s);
                writer.write<int32_t>("i", object.i);
                writer.write<int64_t>("l", object.l);
                writer.write<float>("f", object.f);
                writer.write<double>("d", object.d);
                writer.write("str", object.str);
                writer.write("utfstr", &object.utfStr);

                writer.write("bb", object.byteVec);
                writer.write("cc", object.cc);
                writer.write("ba", object.ba);
                writer.write("ss", object.ss);
                writer.write("ii", object.ii);
                writer.write("ff", object.ff);
                writer.write("dd", object.dd);

                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeObject<byte>(&object.by);
                out.writeObject<char>(&object.c);
                out.writeObject<bool>(&object.boolean);
                out.writeObject<int16_t>(&object.s);
                out.writeObject<int32_t>(&object.i);
                out.writeObject<float>(&object.f);
                out.writeObject<double>(&object.d);
                out.writeObject<std::string>(&object.str);
                out.writeObject<std::string>(&object.utfStr);
            }

            test::Employee hz_serializer<test::Employee>::readPortable(PortableReader &reader) {
                test::Employee employee;
                employee.name = reader.read<std::string>("n");
                employee.age = reader.read<int32_t>("a");
                employee.by = reader.read<byte>("b");
                employee.c = reader.read<char>("c");
                employee.boolean = reader.read<bool>("bo");
                employee.s = reader.read<int16_t>("s");
                employee.i = reader.read<int32_t>("i");
                employee.l = reader.read<int64_t>("l");
                employee.f = reader.read<float>("f");
                employee.d = reader.read<double>("d");
                employee.str = reader.read<std::string>("str");
                employee.utfStr = reader.read<std::string>("utfstr");
                employee.byteVec = *reader.read<std::vector<byte>>("bb");
                employee.cc = *reader.read<std::vector<char>>("cc");
                employee.ba = *reader.read<std::vector<bool>>("ba");
                employee.ss = *reader.read<std::vector<int16_t>>("ss");
                employee.ii = *reader.read<std::vector<int32_t>>("ii");
                employee.ff = *reader.read<std::vector<float>>("ff");
                employee.dd = *reader.read<std::vector<double>>("dd");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                employee.by = *in.readObject<byte>();
                employee.c = *in.readObject<char>();
                employee.boolean = *in.readObject<bool>();
                employee.s = *in.readObject<int16_t>();
                employee.i = *in.readObject<int32_t>();
                employee.f = *in.readObject<float>();
                employee.d = *in.readObject<double>();
                employee.str = *in.readObject<std::string>();
                employee.utfStr = *in.readObject<std::string>();
                return employee;
            }

            int32_t hz_serializer<test::EmployeeEntryComparator>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::EMPLOYEE_FACTORY);
            }

            int32_t hz_serializer<test::EmployeeEntryComparator>::getClassId() {
                return 4;
            }

            void hz_serializer<test::EmployeeEntryComparator>::writeData(const test::EmployeeEntryComparator &object,
                                                                         ObjectDataOutput &writer) {}

            test::EmployeeEntryComparator
            hz_serializer<test::EmployeeEntryComparator>::readData(ObjectDataInput &reader) {
                return test::EmployeeEntryComparator();
            }

            int32_t hz_serializer<test::EmployeeEntryKeyComparator>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::EMPLOYEE_FACTORY);
            }

            int32_t hz_serializer<test::EmployeeEntryKeyComparator>::getClassId() {
                return 5;
            }

            void hz_serializer<test::EmployeeEntryKeyComparator>::writeData(const test::EmployeeEntryKeyComparator &,
                                                                            ObjectDataOutput &) {}

            test::EmployeeEntryKeyComparator hz_serializer<test::EmployeeEntryKeyComparator>::readData(ObjectDataInput &) {
                return test::EmployeeEntryKeyComparator();
            }

            int32_t hz_serializer<test::TestMainPortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestMainPortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_MAIN_PORTABLE);
            }

            void hz_serializer<test::TestMainPortable>::writePortable(const test::TestMainPortable &object,
                                                                      serialization::PortableWriter &writer) {
                writer.write<byte>("b", object.b);
                writer.write("bool", object.boolean);
                writer.write("c", object.c);
                writer.write<int16_t>("s", object.s);
                writer.write<int32_t>("i", object.i);
                writer.write<int64_t>("l", object.l);
                writer.write<float>("f", object.f);
                writer.write<double>("d", object.d);
                writer.write("str", object.str);
                writer.writePortable("p", &object.p);
            }

            test::TestMainPortable
            hz_serializer<test::TestMainPortable>::readPortable(serialization::PortableReader &reader) {
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
                auto innerPortable = reader.readPortable<test::TestInnerPortable>("p");
                if (innerPortable.has_value())
                    object.p = std::move(innerPortable).value();

                return object;
            }

            int32_t hz_serializer<test::TestRawDataPortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestRawDataPortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_RAW_DATA_PORTABLE);
            }

            void hz_serializer<test::TestRawDataPortable>::writePortable(const test::TestRawDataPortable &object,
                                                                         serialization::PortableWriter &writer) {
                writer.write("l", object.l);
                writer.write("c", object.c);
                writer.writePortable("p", &object.p);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.write<int32_t>(object.k);
                out.write(object.s);
                hz_serializer<test::TestDataSerializable>::writeData(object.ds, out);
            }

            test::TestRawDataPortable
            hz_serializer<test::TestRawDataPortable>::readPortable(serialization::PortableReader &reader) {
                test::TestRawDataPortable object;
                object.l = reader.read<int64_t>("l");
                object.c = std::move(reader.read<std::vector<char>>("c")).value();
                auto namedPortable = reader.readPortable<test::TestNamedPortable>("p");
                if (namedPortable.has_value())
                    object.p = namedPortable.value();
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                object.k = in.read<int32_t>();
                object.s = in.read<std::string>();
                object.ds = hz_serializer<test::TestDataSerializable>::readData(in);
                return object;
            }

            int32_t hz_serializer<test::TestDataSerializable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_DATA_FACTORY);
            }

            int32_t hz_serializer<test::TestDataSerializable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_DATA_SERIALIZABLE);
            }

            void
            hz_serializer<test::TestDataSerializable>::writeData(const test::TestDataSerializable &object,
                                                                 serialization::ObjectDataOutput &out) {
                out.write<int32_t>(object.i);
                out.write(object.c);
            }

            test::TestDataSerializable
            hz_serializer<test::TestDataSerializable>::readData(serialization::ObjectDataInput &in) {
                return test::TestDataSerializable{in.read<int32_t>(), in.read<char>()};
            }

            int32_t hz_serializer<test::TestNamedPortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_NAMED_PORTABLE);
            }

            void hz_serializer<test::TestNamedPortable>::writePortable(const test::TestNamedPortable &object,
                                                                       PortableWriter &writer) {
                writer.write("name", object.name);
                writer.write<int32_t>("myint", object.k);
            }

            test::TestNamedPortable hz_serializer<test::TestNamedPortable>::readPortable(PortableReader &reader) {
                return test::TestNamedPortable{reader.read<std::string>("name"), reader.read<int32_t>("myint")};
            }

            int32_t hz_serializer<test::TestInnerPortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInnerPortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_INNER_PORTABLE);
            }

            void hz_serializer<test::TestInnerPortable>::writePortable(const test::TestInnerPortable &object,
                                                                       PortableWriter &writer) {
                writer.write("b", object.bb);
                writer.write("ba", object.ba);
                writer.write("c", object.cc);
                writer.write("s", object.ss);
                writer.write("i", object.ii);
                writer.write("l", object.ll);
                writer.write("f", object.ff);
                writer.write("d", object.dd);
                writer.writePortableArray("nn", &object.nn);
            }

            test::TestInnerPortable hz_serializer<test::TestInnerPortable>::readPortable(PortableReader &reader) {
                test::TestInnerPortable object;
                object.bb = *reader.read<std::vector<byte>>("b");
                object.ba = *reader.read<std::vector<bool>>("ba");
                object.cc = *reader.read<std::vector<char>>("c");
                object.ss = *reader.read<std::vector<int16_t>>("s");
                object.ii = *reader.read<std::vector<int32_t>>("i");
                object.ll = *reader.read<std::vector<int64_t>>("l");
                object.ff = *reader.read<std::vector<float>>("f");
                object.dd = *reader.read<std::vector<double>>("d");
                object.nn = *reader.readPortableArray<test::TestNamedPortable>("nn");
                return object;
            }

            int32_t hz_serializer<test::TestNamedPortableV2>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortableV2>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_NAMED_PORTABLE_2);
            }

            void hz_serializer<test::TestNamedPortableV2>::writePortable(const test::TestNamedPortableV2 &object,
                                                                         PortableWriter &writer) {
                writer.write("name", object.name);
                writer.write<int32_t>("myint", object.k);
                writer.write<int32_t>("v", object.v);
            }

            test::TestNamedPortableV2 hz_serializer<test::TestNamedPortableV2>::readPortable(PortableReader &reader) {
                return test::TestNamedPortableV2{reader.read<std::string>("name"), reader.read<int32_t>("myint"),
                                                 reader.read<int32_t>("v")};
            }

            int32_t hz_serializer<test::TestNamedPortableV3>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestNamedPortableV3>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_NAMED_PORTABLE_3);
            }

            void hz_serializer<test::TestNamedPortableV3>::writePortable(const test::TestNamedPortableV3 &object,
                                                                         PortableWriter &writer) {
                writer.write("name", object.name);
                writer.write<int16_t>("myint", object.k);
            }

            test::TestNamedPortableV3 hz_serializer<test::TestNamedPortableV3>::readPortable(PortableReader &reader) {
                return test::TestNamedPortableV3{reader.read<std::string>("name"), reader.read<int16_t>("myint")};
            }

            int32_t hz_serializer<test::TestInvalidWritePortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInvalidWritePortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_INVALID_WRITE_PORTABLE);
            }

            void
            hz_serializer<test::TestInvalidWritePortable>::writePortable(const test::TestInvalidWritePortable &object,
                                                                         PortableWriter &writer) {
                writer.write<int64_t>("l", object.l);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.write<int32_t>(object.i);
                writer.write("s", object.s);
            }

            test::TestInvalidWritePortable
            hz_serializer<test::TestInvalidWritePortable>::readPortable(PortableReader &reader) {
                return test::TestInvalidWritePortable{reader.read<int64_t>("l"), reader.read<int32_t>("i"),
                                                      reader.read<std::string>("s")};
            }

            int32_t hz_serializer<test::TestInvalidReadPortable>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::TestInvalidReadPortable>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_INVALID_READ_PORTABLE);
            }

            void
            hz_serializer<test::TestInvalidReadPortable>::writePortable(const test::TestInvalidReadPortable &object,
                                                                         PortableWriter &writer) {
                writer.write<int64_t>("l", object.l);
                writer.write<int32_t>("i", object.i);
                writer.write("s", object.s);
            }

            test::TestInvalidReadPortable
            hz_serializer<test::TestInvalidReadPortable>::readPortable(PortableReader &reader) {
                return test::TestInvalidReadPortable{reader.read<int64_t>("l"), reader.read<int32_t>("i"),
                                                      reader.read<std::string>("s")};
            }

            void hz_serializer<test::TestCustomPerson>::write(const test::TestCustomPerson &object, ObjectDataOutput &out) {
                out.write<int32_t>(999);
                out.write(object.name);
                out.write<int32_t>(999);
            }

            test::TestCustomPerson hz_serializer<test::TestCustomPerson>::read(ObjectDataInput &in) {
                assert(999 ==in.read<int32_t>());
                test::TestCustomPerson object{in.read<std::string>()};
                assert(999 == in.read<int32_t>());
                return object;
            }

            void hz_serializer<test::TestCustomXSerializable>::write(const test::TestCustomXSerializable &object, ObjectDataOutput &out) {
                out.write<int32_t>(666);
                out.write(object.id);
                out.write<int32_t>(666);
            }

            test::TestCustomXSerializable hz_serializer<test::TestCustomXSerializable>::read(ObjectDataInput &in) {
                assert(666 ==in.read<int32_t>());
                test::TestCustomXSerializable object{in.read<int32_t>()};
                assert(666 ==in.read<int32_t>());
                return object;
            }

            int32_t hz_serializer<test::ChildTemplatedPortable1>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::ChildTemplatedPortable1>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::CHILD_TEMPLATED_PORTABLE_1);
            }

            void
            hz_serializer<test::ChildTemplatedPortable1>::writePortable(const test::ChildTemplatedPortable1 &object,
                                                                         PortableWriter &out) {
                out.write("s1", object.s1);
                out.write("s1", object.s2);
            }

            test::ChildTemplatedPortable1
            hz_serializer<test::ChildTemplatedPortable1>::readPortable(PortableReader &reader) {
                return test::ChildTemplatedPortable1{reader.read<std::string>("s1"), reader.read<std::string>("s2")};
            }

            int32_t hz_serializer<test::ChildTemplatedPortable2>::getFactoryId() {
                return static_cast<int32_t>(test::TestSerializationConstants::TEST_PORTABLE_FACTORY);
            }

            int32_t hz_serializer<test::ChildTemplatedPortable2>::getClassId() {
                return static_cast<int32_t>(test::TestSerializationConstants::CHILD_TEMPLATED_PORTABLE_2);
            }

            void
            hz_serializer<test::ChildTemplatedPortable2>::writePortable(const test::ChildTemplatedPortable2 &object,
                                                                         PortableWriter &out) {
                out.write("s1", object.s1);
            }

            test::ChildTemplatedPortable2
            hz_serializer<test::ChildTemplatedPortable2>::readPortable(PortableReader &reader) {
                return test::ChildTemplatedPortable2{reader.read<std::string>("s1")};
            }

        }
    }
}

