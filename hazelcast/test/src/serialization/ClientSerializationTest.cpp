/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 8/27/13.

#include <stdint.h>
#include <gtest/gtest.h>

#include "customSerialization/TestCustomSerializerX.h"
#include "customSerialization/TestCustomXSerializable.h"
#include "customSerialization/TestCustomPersonSerializer.h"
#include "serialization/TestNamedPortableV2.h"
#include "serialization/TestRawDataPortable.h"
#include "serialization/TestInvalidReadPortable.h"
#include "serialization/TestInvalidWritePortable.h"
#include "serialization/ChildTemplatedPortable2.h"
#include "serialization/ParentTemplatedPortable.h"
#include "serialization/ChildTemplatedPortable1.h"
#include "serialization/ObjectCarryingPortable.h"
#include "serialization/TestInnerPortable.h"
#include "serialization/TestMainPortable.h"
#include "TestNamedPortableV3.h"

#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/util/MurmurHash3.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientSerializationTest : public ::testing::Test {
            protected:
                class NonSerializableObject {};

                class DummyGlobalSerializer : public serialization::StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const {
                        return 123;
                    }

                    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
                        std::string value("Dummy string");
                        out.writeUTF(&value);
                    }

                    virtual void *read(serialization::ObjectDataInput &in) {
                        return in.readUTF().release();
                    }
                };

                template<typename T>
                T toDataAndBackToObject(serialization::pimpl::SerializationService& ss, T& value) {
                    serialization::pimpl::Data data = ss.toData<T>(&value);
                    return *(ss.toObject<T>(data));
                }

                static const unsigned int LARGE_ARRAY_SIZE;
            };

            const unsigned int ClientSerializationTest::LARGE_ARRAY_SIZE =
                    1 * 1024 * 1024;   // 1 MB. Previously it was 10 MB but then the
                                        // test fails when using Windows 32-bit DLL
                                        // library with std::bad_alloc with 10 MB

            TEST_F(ClientSerializationTest, testCustomSerialization) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                boost::shared_ptr<serialization::StreamSerializer> serializer1(
                        new TestCustomSerializerX<TestCustomXSerializable>());
                boost::shared_ptr<serialization::StreamSerializer> serializer2(new TestCustomPersonSerializer());

                serializationService.registerSerializer(serializer1);
                serializationService.registerSerializer(serializer2);

                TestCustomXSerializable a(131321);
                serialization::pimpl::Data data = serializationService.toData<TestCustomXSerializable>(&a);
                std::auto_ptr<TestCustomXSerializable> a2 = serializationService.toObject<TestCustomXSerializable>(
                        data);
                ASSERT_EQ(a, *a2);

                TestCustomPerson b("TestCustomPerson");
                serialization::pimpl::Data data1 = serializationService.toData<TestCustomPerson>(&b);
                std::auto_ptr<TestCustomPerson> b2 = serializationService.toObject<TestCustomPerson>(data1);
                ASSERT_EQ(b, *b2);
            }


            TEST_F(ClientSerializationTest, testRawData) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestDataSerializable ds(123, 's');
                TestNamedPortable np("named portable", 34567);
                TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);

                serialization::pimpl::Data data = serializationService.toData<TestRawDataPortable>(&p);
                std::auto_ptr<TestRawDataPortable> x = serializationService.toObject<TestRawDataPortable>(data);
                ASSERT_EQ(p, *x);
            }

            TEST_F(ClientSerializationTest, testIdentifiedDataSerializable) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;
                TestDataSerializable np(4, 'k');
                data = serializationService.toData<TestDataSerializable>(&np);

                std::auto_ptr<TestDataSerializable> tnp1;
                tnp1 = serializationService.toObject<TestDataSerializable>(data);

                ASSERT_EQ(np, *tnp1);
                int x = 4;
                data = serializationService.toData<int>(&x);
                std::auto_ptr<int> ptr = serializationService.toObject<int>(data);
                int y = *ptr;
                ASSERT_EQ(x, y);
            }

            TEST_F(ClientSerializationTest, testRawDataWithoutRegistering) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestNamedPortable np("named portable", 34567);
                TestDataSerializable ds(123, 's');
                TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);

                serialization::pimpl::Data data = serializationService.toData<TestRawDataPortable>(&p);
                std::auto_ptr<TestRawDataPortable> x = serializationService.toObject<TestRawDataPortable>(data);
                ASSERT_EQ(p, *x);

            }

            TEST_F(ClientSerializationTest, testInvalidWrite) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidWritePortable p(2131, 123, "q4edfd");
                ASSERT_THROW(serializationService.toData<TestInvalidWritePortable>(&p),
                             exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testInvalidRead) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidReadPortable p(2131, 123, "q4edfd");
                serialization::pimpl::Data data = serializationService.toData<TestInvalidReadPortable>(&p);
                ASSERT_THROW(serializationService.toObject<TestInvalidReadPortable>(data),
                             exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testDifferentVersions) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                TestNamedPortable p1("portable-v1", 111);
                serialization::pimpl::Data data = serializationService.toData<TestNamedPortable>(&p1);

                TestNamedPortableV2 p2("portable-v2", 123);
                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV2>(&p2);

                std::auto_ptr<TestNamedPortableV2> t2 = serializationService2.toObject<TestNamedPortableV2>(data);
                ASSERT_EQ(std::string("portable-v1"), t2->name);
                ASSERT_EQ(111, t2->k);
                ASSERT_EQ(0, t2->v);

                std::auto_ptr<TestNamedPortable> t1 = serializationService.toObject<TestNamedPortable>(data2);
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123 * 10, t1->k);

            }

            TEST_F(ClientSerializationTest, testBasicFunctionality) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);

                std::auto_ptr<int> returnedInt = serializationService.toObject<int>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.toData<int16_t>(&f);

                std::auto_ptr<int16_t> temp = serializationService.toObject<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                std::auto_ptr<TestNamedPortable> tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable>(data);

                tnp2 = serializationService.toObject<TestNamedPortable>(data);

                ASSERT_EQ(np, *tnp1);
                ASSERT_EQ(np, *tnp2);

                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                bool boolArray[] = {false, true, true, false};
                std::vector<bool> ba(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                TestNamedPortable portableArray[5];
                for (int i = 0; i < 5; i++) {
                    portableArray[i].name = "named-portable-" + util::IOUtil::to_string(i);
                    portableArray[i].k = i;
                }
                std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

                TestInnerPortable inner(bb, ba, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                std::auto_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);

                tip2 = serializationService.toObject<TestInnerPortable>(data);

                ASSERT_EQ(inner, *tip1);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main((byte) 113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                std::auto_ptr<TestMainPortable> tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable>(data);

                tmp2 = serializationService.toObject<TestMainPortable>(data);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithLargeData) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                byte *byteArray = new byte[LARGE_ARRAY_SIZE];
                std::vector<byte> bb(byteArray, byteArray + LARGE_ARRAY_SIZE);
                bool *boolArray = new bool[LARGE_ARRAY_SIZE];
                std::vector<bool> ba(boolArray, boolArray + LARGE_ARRAY_SIZE);
                char *charArray;
                charArray = new char[LARGE_ARRAY_SIZE];
                std::vector<char> cc(charArray, charArray + LARGE_ARRAY_SIZE);
                int16_t *shortArray;
                shortArray = new int16_t[LARGE_ARRAY_SIZE];
                std::vector<int16_t> ss(shortArray, shortArray + LARGE_ARRAY_SIZE);
                int32_t *integerArray;
                integerArray = new int32_t[LARGE_ARRAY_SIZE];
                std::vector<int32_t> ii(integerArray, integerArray + LARGE_ARRAY_SIZE);
                int64_t *longArray;
                longArray = new int64_t[LARGE_ARRAY_SIZE];
                std::vector<int64_t> ll(longArray, longArray + LARGE_ARRAY_SIZE);
                float *floatArray;
                floatArray = new float[LARGE_ARRAY_SIZE];
                std::vector<float> ff(floatArray, floatArray + LARGE_ARRAY_SIZE);
                double *doubleArray;
                doubleArray = new double[LARGE_ARRAY_SIZE];
                std::vector<double> dd(doubleArray, doubleArray + LARGE_ARRAY_SIZE);

                TestNamedPortable portableArray[5];

                for (int i = 0; i < 5; i++) {
                    portableArray[i].name = "named-portable-" + util::IOUtil::to_string(i);
                    portableArray[i].k = i;
                }
                std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

                TestInnerPortable inner(bb, ba, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                std::auto_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);

                tip2 = serializationService.toObject<TestInnerPortable>(data);

                ASSERT_EQ(inner, *tip1);
                ASSERT_EQ(inner, *tip2);

            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithDifferentVersions) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);
                serialization::pimpl::Data data;

                int32_t x = 3;
                data = serializationService.toData<int32_t>(&x);

                std::auto_ptr<int32_t> returnedInt = serializationService.toObject<int32_t>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.toData<int16_t>(&f);

                std::auto_ptr<int16_t> temp = serializationService.toObject<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                std::auto_ptr<TestNamedPortable> tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable>(data);

                tnp2 = serializationService2.toObject<TestNamedPortable>(data);

                ASSERT_EQ(np, *tnp1);
                ASSERT_EQ(np, *tnp2);

                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                bool boolArray[] = {true, true, false};
                std::vector<bool> ba(boolArray, boolArray + 3);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                TestNamedPortable portableArray[5];
                for (int i = 0; i < 5; i++) {
                    portableArray[i].name = "named-portable-" + util::IOUtil::to_string(i);
                    portableArray[i].k = i;
                }
                std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

                TestInnerPortable inner(bb, ba, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                std::auto_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);

                tip2 = serializationService2.toObject<TestInnerPortable>(data);

                ASSERT_EQ(inner, *tip1);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main((byte) 113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                std::auto_ptr<TestMainPortable> tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable>(data);

                tmp2 = serializationService2.toObject<TestMainPortable>(data);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testTemplatedPortable_whenMultipleTypesAreUsed) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ParentTemplatedPortable<ChildTemplatedPortable1> portable(new ChildTemplatedPortable1("aaa", "bbb"));
                ss.toData<ParentTemplatedPortable<ChildTemplatedPortable1> >(&portable);
                ParentTemplatedPortable<ChildTemplatedPortable2> portable2(new ChildTemplatedPortable2("ccc"));

                ASSERT_THROW(ss.toData<ParentTemplatedPortable<ChildTemplatedPortable2> >(&portable2),
                             exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testDataHash) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                std::string serializable = "key1";
                serialization::pimpl::Data data = serializationService.toData<std::string>(&serializable);
                serialization::pimpl::Data data2 = serializationService.toData<std::string>(&serializable);
                ASSERT_EQ(data.getPartitionHash(), data2.getPartitionHash());

            }

            TEST_F(ClientSerializationTest, testPrimitives) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                ASSERT_EQ(by, toDataAndBackToObject(serializationService, by));
                ASSERT_EQ(boolean, toDataAndBackToObject(serializationService, boolean));
                ASSERT_EQ(c, toDataAndBackToObject(serializationService, c));
                ASSERT_EQ(s, toDataAndBackToObject(serializationService, s));
                ASSERT_EQ(i, toDataAndBackToObject(serializationService, i));
                ASSERT_EQ(l, toDataAndBackToObject(serializationService, l));
                ASSERT_EQ(f, toDataAndBackToObject(serializationService, f));
                ASSERT_EQ(d, toDataAndBackToObject(serializationService, d));
                ASSERT_EQ(str, toDataAndBackToObject(serializationService, str));
                ASSERT_EQ(utfStr, toDataAndBackToObject(serializationService, utfStr));
            }

            TEST_F(ClientSerializationTest, testPrimitiveArrays) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                std::vector<bool> ba(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                const std::string stringArray[] = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};
                std::vector<std::string> stringVector;
                for (int i = 0; i < 3; ++i) {
                    stringVector.push_back(stringArray[i]);
                }

                ASSERT_EQ(cc, toDataAndBackToObject<std::vector<char> >(serializationService, cc));
                ASSERT_EQ(ba, toDataAndBackToObject<std::vector<bool> >(serializationService, ba));
                ASSERT_EQ(bb, toDataAndBackToObject<std::vector<byte> >(serializationService, bb));
                ASSERT_EQ(ss, toDataAndBackToObject<std::vector<int16_t> >(serializationService, ss));
                ASSERT_EQ(ii, toDataAndBackToObject<std::vector<int32_t> >(serializationService, ii));
                ASSERT_EQ(ll, toDataAndBackToObject<std::vector<int64_t> >(serializationService, ll));
                ASSERT_EQ(ff, toDataAndBackToObject<std::vector<float> >(serializationService, ff));
                ASSERT_EQ(dd, toDataAndBackToObject<std::vector<double> >(serializationService, dd));
                ASSERT_EQ(stringVector, toDataAndBackToObject<std::vector<std::string> >(serializationService,
                                                                                         stringVector));
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithPortable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                TestNamedPortable *namedPortable = new TestNamedPortable("name", 2);
                ObjectCarryingPortable<TestNamedPortable> objectCarryingPortable(namedPortable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestNamedPortable> >(
                        &objectCarryingPortable);
                std::auto_ptr<ObjectCarryingPortable<TestNamedPortable> > ptr = ss.toObject<ObjectCarryingPortable<TestNamedPortable> >(
                        data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithIdentifiedDataSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                TestDataSerializable *testDataSerializable = new TestDataSerializable(2, 'c');
                ObjectCarryingPortable<TestDataSerializable> objectCarryingPortable(testDataSerializable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestDataSerializable> >(
                        &objectCarryingPortable);
                std::auto_ptr<ObjectCarryingPortable<TestDataSerializable> > ptr = ss.toObject<ObjectCarryingPortable<TestDataSerializable> >(
                        data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomXSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                boost::shared_ptr<serialization::StreamSerializer> serializer(
                        new TestCustomSerializerX<TestCustomXSerializable>());

                ss.registerSerializer(serializer);

                TestCustomXSerializable *customXSerializable = new TestCustomXSerializable(131321);
                ObjectCarryingPortable<TestCustomXSerializable> objectCarryingPortable(customXSerializable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestCustomXSerializable> >(
                        &objectCarryingPortable);
                std::auto_ptr<ObjectCarryingPortable<TestCustomXSerializable> > ptr = ss.toObject<ObjectCarryingPortable<TestCustomXSerializable> >(
                        data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomPersonSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                boost::shared_ptr<serialization::StreamSerializer> serializer(new TestCustomPersonSerializer());

                ss.registerSerializer(serializer);

                TestCustomPerson *testCustomPerson = new TestCustomPerson("TestCustomPerson");

                ObjectCarryingPortable<TestCustomPerson> objectCarryingPortable(testCustomPerson);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestCustomPerson> >(
                        &objectCarryingPortable);
                std::auto_ptr<ObjectCarryingPortable<TestCustomPerson> > ptr = ss.toObject<ObjectCarryingPortable<TestCustomPerson> >(
                        data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }


            TEST_F(ClientSerializationTest, testNullData) {
                serialization::pimpl::Data data;
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                std::auto_ptr<int32_t> ptr = ss.toObject<int32_t>(data);
                ASSERT_EQ(ptr.get(), (int32_t *)NULL);
            }

            TEST_F(ClientSerializationTest, testMorphingWithDifferentTypes_differentVersions) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                TestNamedPortableV3 p2("portable-v2", 123);
                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV3>(&p2);

                std::auto_ptr<TestNamedPortable> t1 = serializationService.toObject<TestNamedPortable>(data2);
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123, t1->k);
            }

            TEST_F(ClientSerializationTest, ObjectDataInputOutput) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::pimpl::DataOutput dataOutput;
                serialization::ObjectDataOutput out(dataOutput, &serializationService.getSerializerHolder());

                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                byte byteArray[] = {50, 100, 150, 200};
                std::vector<byte> byteVec(byteArray, byteArray + 4);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                std::vector<bool> ba(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                const std::string stringArray[] = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};
                std::vector<std::string *> stringVector;
                for (int i = 0; i < 3; ++i) {
                    stringVector.push_back(new std::string(stringArray[i]));
                }

                out.writeByte(by);
                out.writeChar(c);
                out.writeBoolean(boolean);
                out.writeShort(s);
                out.writeInt(i);
                out.writeLong(l);
                out.writeFloat(f);
                out.writeDouble(d);
                out.writeUTF(&str);
                out.writeUTF(&utfStr);

                out.writeByteArray(&byteVec);
                out.writeCharArray(&cc);
                out.writeBooleanArray(&ba);
                out.writeShortArray(&ss);
                out.writeIntArray(&ii);
                out.writeFloatArray(&ff);
                out.writeDoubleArray(&dd);
                out.writeUTFArray(&stringVector);

                out.writeObject<byte>(&by);
                out.writeObject<char>(&c);
                out.writeObject<bool>(&boolean);
                out.writeObject<int16_t>(&s);
                out.writeObject<int32_t>(&i);
                out.writeObject<float>(&f);
                out.writeObject<double>(&d);
                out.writeObject<std::string>(&str);
                out.writeObject<std::string>(&utfStr);

                std::auto_ptr<std::vector<byte> > buffer = dataOutput.toByteArray();
                serialization::pimpl::DataInput dataInput(*buffer);
                serialization::ObjectDataInput in(dataInput, serializationService.getSerializerHolder());

                ASSERT_EQ(by, in.readByte());
                ASSERT_EQ(c, in.readChar());
                ASSERT_EQ(boolean, in.readBoolean());
                ASSERT_EQ(s, in.readShort());
                ASSERT_EQ(i, in.readInt());
                ASSERT_EQ(l, in.readLong());
                ASSERT_FLOAT_EQ(f, in.readFloat());
                ASSERT_DOUBLE_EQ(d, in.readDouble());
                ASSERT_EQ(str, *in.readUTF());
                ASSERT_EQ(utfStr, *in.readUTF());

                ASSERT_EQ(byteVec, *in.readByteArray());
                ASSERT_EQ(cc, *in.readCharArray());
                ASSERT_EQ(ba, *in.readBooleanArray());
                ASSERT_EQ(ss, *in.readShortArray());
                ASSERT_EQ(ii, *in.readIntArray());
                ASSERT_EQ(ff, *in.readFloatArray());
                ASSERT_EQ(dd, *in.readDoubleArray());
                std::auto_ptr<std::vector<std::string> > strArrRead = in.readUTFArray();
                ASSERT_NE((std::vector<std::string> *)NULL, strArrRead.get());
                ASSERT_EQ(stringVector.size(), strArrRead->size());
                for (size_t j = 0; j < stringVector.size(); ++j) {
                    ASSERT_EQ((*strArrRead)[j], *(stringVector[j]));
                }

                ASSERT_EQ(by, *in.readObject<byte>());
                ASSERT_EQ(c, *in.readObject<char>());
                ASSERT_EQ(boolean, *in.readObject<bool>());
                ASSERT_EQ(s, *in.readObject<int16_t>());
                ASSERT_EQ(i, *in.readObject<int32_t>());
                ASSERT_EQ(f, *in.readObject<float>());
                ASSERT_EQ(d, *in.readObject<double>());
                ASSERT_EQ(str, *in.readObject<std::string>());
                ASSERT_EQ(utfStr, *in.readObject<std::string>());

            }

            TEST_F(ClientSerializationTest, testGetUTF8CharCount) {
                std::string utfStr = "xyzä123";

                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::pimpl::DataOutput dataOutput;
                serialization::ObjectDataOutput out(dataOutput, &serializationService.getSerializerHolder());


                out.writeUTF(&utfStr);
                std::auto_ptr<std::vector<byte> > byteArray = out.toByteArray();
                int strLen = util::Bits::readIntB(*byteArray, 0);
                ASSERT_EQ(7, strLen);
            }

            TEST_F(ClientSerializationTest, testGlobalSerializer) {
                SerializationConfig serializationConfig;

                serializationConfig.setGlobalSerializer(
                        boost::shared_ptr<serialization::StreamSerializer>(new DummyGlobalSerializer()));
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                
                NonSerializableObject obj;

                serialization::pimpl::Data data = serializationService.toData<NonSerializableObject>(&obj);

                std::auto_ptr<std::string> deserializedValue = serializationService.toObject<std::string>(data);
                ASSERT_NE((std::string *) NULL, deserializedValue.get());
                ASSERT_EQ("Dummy string", *deserializedValue);
            }
        }
    }
}

