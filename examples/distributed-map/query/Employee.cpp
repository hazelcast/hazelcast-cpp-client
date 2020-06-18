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
#include "Employee.h"

namespace hazelcast {
    namespace client {
        namespace examples {
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
        }

        namespace serialization {
            int32_t hz_serializer<examples::Employee>::getFactoryId() {
                return 666;
            }

            int32_t hz_serializer<examples::Employee>::getClassId() {
                return 2;
            }

            void hz_serializer<examples::Employee>::writePortable(const examples::Employee &object, PortableWriter &writer) {
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
                out.writeObject<char>(object.c);
                out.writeObject<bool>(&object.boolean);
                out.writeObject<int16_t>(&object.s);
                out.writeObject<int32_t>(&object.i);
                out.writeObject<float>(&object.f);
                out.writeObject<double>(&object.d);
                out.writeObject<std::string>(&object.str);
                out.writeObject<std::string>(&object.utfStr);
            }

            examples::Employee hz_serializer<examples::Employee>::readPortable(PortableReader &reader) {
                examples::Employee employee;
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

            int32_t hz_serializer<examples::EmployeeEntryComparator>::getFactoryId() {
                return static_cast<int32_t>(1);
            }

            int32_t hz_serializer<examples::EmployeeEntryComparator>::getClassId() {
                return 4;
            }

            void hz_serializer<examples::EmployeeEntryComparator>::writeData(const examples::EmployeeEntryComparator &object,
                                                                         ObjectDataOutput &writer) {}

            examples::EmployeeEntryComparator
            hz_serializer<examples::EmployeeEntryComparator>::readData(ObjectDataInput &reader) {
                return examples::EmployeeEntryComparator();
            }

        }
    }
}

