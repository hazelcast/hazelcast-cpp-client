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
            Employee::Employee() = default;

            Employee::Employee(std::string name, int32_t age) : age_(age), name_(name) {
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

            bool Employee::operator==(const Employee &rhs) const {
                return age_ == rhs.get_age() && name_ == rhs.get_name();
            }

            bool Employee::operator!=(const Employee &employee) const {
                return !(*this == employee);
            }

            int32_t Employee::get_age() const {
                return age_;
            }

            const std::string &Employee::get_name() const {
                return name_;
            }

            bool Employee::operator<(const Employee &rhs) const {
                return age_ < rhs.get_age();
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
                out << "Employee:[" << employee.get_name() << ", " << employee.get_age() << "]";
                return out;
            }
        }

        namespace serialization {
            int32_t hz_serializer<examples::Employee>::get_factory_id() {
                return 666;
            }

            int32_t hz_serializer<examples::Employee>::get_class_id() {
                return 2;
            }

            void hz_serializer<examples::Employee>::write_portable(const examples::Employee &object, PortableWriter &writer) {
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

                serialization::ObjectDataOutput &out = writer.get_raw_data_output();
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

            examples::Employee hz_serializer<examples::Employee>::read_portable(PortableReader &reader) {
                examples::Employee employee;
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
                serialization::ObjectDataInput &in = reader.get_raw_data_input();
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

            int32_t hz_serializer<examples::EmployeeEntryComparator>::get_factory_id() {
                return static_cast<int32_t>(1);
            }

            int32_t hz_serializer<examples::EmployeeEntryComparator>::get_class_id() {
                return 4;
            }

            void hz_serializer<examples::EmployeeEntryComparator>::write_data(const examples::EmployeeEntryComparator &object,
                                                                         ObjectDataOutput &writer) {}

            examples::EmployeeEntryComparator
            hz_serializer<examples::EmployeeEntryComparator>::read_data(ObjectDataInput &reader) {
                return examples::EmployeeEntryComparator();
            }

        }
    }
}

