/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 11/11/13.

#include "Employee.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "TestSerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace test {
            Employee::Employee():age(-1), name("") {
            }

            Employee::Employee(std::string name, int32_t age)
                    :age(age)
                    , name(name) {
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
                int64_t  longArray[] = {0, 1, 5, 7, 9, 11};
                ll = std::vector<int64_t >(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                ff = std::vector<float>(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                dd = std::vector<double>(doubleArray, doubleArray + 3);
            }

            bool Employee::operator==(const Employee &rhs) const {
                return age == rhs.getAge() && name == rhs.getName();
            }

            bool Employee::operator !=(const Employee &employee) const {
                return !(*this == employee);
            }

            int32_t Employee::getFactoryId() const {
                return 666;
            }

            int32_t Employee::getClassId() const {
                return 2;
            }

            void Employee::writePortable(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", &name);
                writer.writeInt("a", age);

                writer.writeByte("b", by);
                writer.writeChar("c", c);
                writer.writeBoolean("bo", boolean);
                writer.writeShort("s", s);
                writer.writeInt("i", i);
                writer.writeLong("l", l);
                writer.writeFloat("f", f);
                writer.writeDouble("d", d);
                writer.writeUTF("str", &str);
                writer.writeUTF("utfstr", &utfStr);

                writer.writeByteArray("bb", &byteVec);
                writer.writeCharArray("cc", &cc);
                writer.writeBooleanArray("ba", &ba);
                writer.writeShortArray("ss", &ss);
                writer.writeIntArray("ii", &ii);
                writer.writeFloatArray("ff", &ff);
                writer.writeDoubleArray("dd", &dd);

                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeObject<byte>(&by);
                out.writeObject<char>(&c);
                out.writeObject<bool>(&boolean);
                out.writeObject<int16_t>(&s);
                out.writeObject<int32_t>(&i);
                out.writeObject<float>(&f);
                out.writeObject<double>(&d);
                out.writeObject<std::string>(&str);
                out.writeObject<std::string>(&utfStr);
            }

            void Employee::readPortable(serialization::PortableReader &reader) {
                name = *reader.readUTF("n");
                age = reader.readInt("a");

                by = reader.readByte("b");;
                c = reader.readChar("c");;
                boolean = reader.readBoolean("bo");;
                s = reader.readShort("s");;
                i = reader.readInt("i");;
                l = reader.readLong("l");;
                f = reader.readFloat("f");;
                d = reader.readDouble("d");;
                str = *reader.readUTF("str");;
                utfStr = *reader.readUTF("utfstr");;

                byteVec = *reader.readByteArray("bb");;
                cc = *reader.readCharArray("cc");;
                ba = *reader.readBooleanArray("ba");;
                ss = *reader.readShortArray("ss");;
                ii = *reader.readIntArray("ii");;
                ff = *reader.readFloatArray("ff");;
                dd = *reader.readDoubleArray("dd");;

                serialization::ObjectDataInput &in = reader.getRawDataInput();
                by = *in.readObject<byte>();
                c = *in.readObject<char>();
                boolean = *in.readObject<bool>();
                s = *in.readObject<int16_t>();
                i = *in.readObject<int32_t>();
                f = *in.readObject<float>();
                d = *in.readObject<double>();
                str = *in.readObject<std::string>();
                utfStr = *in.readObject<std::string>();
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

            int32_t EmployeeEntryComparator::getFactoryId() const {
                return 666;
            }

            int32_t EmployeeEntryComparator::getClassId() const {
                return 4;
            }

            void EmployeeEntryComparator::writeData(serialization::ObjectDataOutput &writer) const {
            }

            void EmployeeEntryComparator::readData(serialization::ObjectDataInput &reader) {
            }

            int EmployeeEntryComparator::compare(const std::pair<const int32_t *, const Employee *> *lhs,
                        const std::pair<const int32_t *, const Employee *> *rhs) const {
                const Employee *lv = lhs->second;
                const Employee *rv = rhs->second;

                if (NULL == lv && NULL == rv) {
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

                if (NULL == lv) {
                    return -1;
                }

                if (NULL == rv) {
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

                if (NULL == key1) {
                    return -1;
                }

                if (NULL == key2) {
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

            int32_t EmployeeEntryKeyComparator::getClassId() const {
                return 5;
            }

            std::ostream &operator<<(std::ostream &out, const Employee &employee) {
                out << "Employee:[" << employee.getName() << ", " << employee.getAge() << "]";
                return out;
            }
        }
    }
}

