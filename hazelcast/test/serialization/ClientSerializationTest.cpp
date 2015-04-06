//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


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
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "serialization/ClientSerializationTest.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/util/MurmurHash3.h"
#include "TestNamedPortableV3.h"

namespace hazelcast {
    namespace client {
        namespace test {
            static const unsigned int LARGE_ARRAY_SIZE = 10 * 1024 * 1024; // 10 MB
            static const unsigned int LARGE_DATA_SIZE_IN_BYTES = 10 * LARGE_ARRAY_SIZE * sizeof(double); // 10 * 10MB * (sizeof(double))

            static serialization::pimpl::Data writeAndReadData(serialization::pimpl::Data& data, serialization::pimpl::SerializationService& ss1, serialization::pimpl::SerializationService& ss2) {

                serialization::pimpl::PortableContext& contextIn = ss1.getPortableContext();
                serialization::pimpl::PortableContext& contextOut = ss2.getPortableContext();
                serialization::pimpl::Packet packet(contextIn, data);

                std::auto_ptr<char> buf(new char[LARGE_DATA_SIZE_IN_BYTES]);
                util::ByteBuffer buffer(buf.get(), LARGE_DATA_SIZE_IN_BYTES);
                iTest::assertTrue(packet.writeTo(buffer));
                buffer.flip();

                serialization::pimpl::Packet p2(contextOut);
                iTest::assertTrue(p2.readFrom(buffer));
                return p2.getData();
            }

            ClientSerializationTest::ClientSerializationTest(int dummy)
            : iTest::iTestFixture<ClientSerializationTest>("ClientSerializationTest") {

            }

            void ClientSerializationTest::beforeClass() {
            }

            void ClientSerializationTest::afterClass() {
            }

            void ClientSerializationTest::beforeTest() {
            }

            void ClientSerializationTest::afterTest() {
            }

            void ClientSerializationTest::addTests() {
                addTest(&ClientSerializationTest::testBasicFunctionality, "testBasicFunctionality");
                addTest(&ClientSerializationTest::testBasicFunctionalityWithLargeData, "testBasicFunctionalityWithLargeData");
                addTest(&ClientSerializationTest::testBasicFunctionalityWithDifferentVersions, "testBasicFunctionalityWithDifferentVersions");
                addTest(&ClientSerializationTest::testCustomSerialization, "testCustomSerialization");
                addTest(&ClientSerializationTest::testRawData, "testRawData");
                addTest(&ClientSerializationTest::testIdentifiedDataSerializable, "testIdentifiedDataSerializable");
                addTest(&ClientSerializationTest::testRawDataWithoutRegistering, "testRawDataWithoutRegistering");
                addTest(&ClientSerializationTest::testInvalidWrite, "testInvalidWrite");
                addTest(&ClientSerializationTest::testInvalidRead, "testInvalidRead");
                addTest(&ClientSerializationTest::testDifferentVersions, "testDifferentVersions");
                addTest(&ClientSerializationTest::testBasicFunctionality, "testBasicFunctionality");
                addTest(&ClientSerializationTest::testTemplatedPortable_whenMultipleTypesAreUsed, "testTemplatedPortable_whenMultipleTypesAreUsed");
                addTest(&ClientSerializationTest::testDataHash, "testDataHash");
                addTest(&ClientSerializationTest::testPrimitives, "testPrimitives");
                addTest(&ClientSerializationTest::testPrimitiveArrays, "testPrimitiveArrays");
                addTest(&ClientSerializationTest::testWriteObjectWithPortable, "testWriteObjectWithPortable");
                addTest(&ClientSerializationTest::testWriteObjectWithIdentifiedDataSerializable, "testWriteObjectWithIdentifiedDataSerilizable");
                addTest(&ClientSerializationTest::testWriteObjectWithCustomXSerializable, "testWriteObjectWithCustomXSerializable");
                addTest(&ClientSerializationTest::testWriteObjectWithCustomPersonSerializable, "testWriteObjectWithCustomPersonSerializable");
                addTest(&ClientSerializationTest::testNullData, "testNullData");
                addTest(&ClientSerializationTest::testMorphingWithDifferentTypes_differentVersions, "testMorphingWithDifferentTypes_differentVersions");
            }

            void ClientSerializationTest::testCustomSerialization() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                boost::shared_ptr<serialization::SerializerBase> serializer1(new TestCustomSerializerX<TestCustomXSerializable>());
                boost::shared_ptr<serialization::SerializerBase> serializer2(new TestCustomPersonSerializer());

                serializationService.registerSerializer(serializer1);
                serializationService.registerSerializer(serializer2);

                TestCustomXSerializable a(131321);
                serialization::pimpl::Data data = serializationService.toData<TestCustomXSerializable>(&a);
                boost::shared_ptr<TestCustomXSerializable> a2 = serializationService.toObject<TestCustomXSerializable>(data);
                iTest::assertEqual(a, *a2);

                TestCustomPerson b("TestCustomPerson");
                serialization::pimpl::Data data1 = serializationService.toData<TestCustomPerson>(&b);
                boost::shared_ptr<TestCustomPerson> b2 = serializationService.toObject<TestCustomPerson>(data1);
                iTest::assertEqual(b, *b2);
            }


            void ClientSerializationTest::testRawData() {
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
                boost::shared_ptr<TestRawDataPortable> x = serializationService.toObject<TestRawDataPortable>(data);
                iTest::assertEqual(p, *x);
            }


            void ClientSerializationTest::testIdentifiedDataSerializable() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;
                TestDataSerializable np(4, 'k');
                data = serializationService.toData<TestDataSerializable>(&np);

                boost::shared_ptr<TestDataSerializable> tnp1;
                tnp1 = serializationService.toObject<TestDataSerializable>(data);

                iTest::assertEqual(np, *tnp1);
                int x = 4;
                data = serializationService.toData<int>(&x);
                boost::shared_ptr<int> ptr = serializationService.toObject<int>(data);
                int y = *ptr;
                iTest::assertEqual(x, y);
            }

            void ClientSerializationTest::testRawDataWithoutRegistering() {
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
                boost::shared_ptr<TestRawDataPortable> x = serializationService.toObject<TestRawDataPortable>(data);
                iTest::assertEqual(p, *x);

            }


            void ClientSerializationTest::testInvalidWrite() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidWritePortable p(2131, 123, "q4edfd");
                try {
                    serializationService.toData<TestInvalidWritePortable>(&p);
                    iTest::assertTrue(false, "toData should trow exception");
                } catch (exception::HazelcastSerializationException&) {

                }
            }

            void ClientSerializationTest::testInvalidRead() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidReadPortable p(2131, 123, "q4edfd");
                serialization::pimpl::Data data = serializationService.toData<TestInvalidReadPortable>(&p);
                try {
                    serializationService.toObject<TestInvalidReadPortable>(data);
                    iTest::assertTrue(false, "toObject should trow exception");
                } catch (exception::HazelcastSerializationException&) {

                }
            }

            void ClientSerializationTest::testDifferentVersions() {
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


                data = writeAndReadData(data, serializationService, serializationService2);
                boost::shared_ptr<TestNamedPortableV2> t2 = serializationService2.toObject<TestNamedPortableV2>(data);
                iTest::assertEqual(std::string("portable-v1"), t2->name);
                iTest::assertEqual(111, t2->k);
                iTest::assertEqual(0, t2->v);

                data2 = writeAndReadData(data2, serializationService2, serializationService);
                boost::shared_ptr<TestNamedPortable> t1 = serializationService.toObject<TestNamedPortable>(data2);
                iTest::assertEqual(std::string("portable-v2"), t1->name);
                iTest::assertEqual(123 * 10, t1->k);

            }

            void ClientSerializationTest::testBasicFunctionality() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);
                data = writeAndReadData(data, serializationService, serializationService);
                boost::shared_ptr<int> returnedInt = serializationService.toObject<int>(data);
                iTest::assertEqual(x, *returnedInt);

                short f = 2;
                data = serializationService.toData<short>(&f);
                data = writeAndReadData(data, serializationService, serializationService);
                boost::shared_ptr<short> temp = serializationService.toObject<short>(data);
                iTest::assertEqual(f, *temp);

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                boost::shared_ptr<TestNamedPortable> tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService);
                tnp2 = serializationService.toObject<TestNamedPortable>(data);

                iTest::assertEqual(np, *tnp1);
                iTest::assertEqual(np, *tnp2);

                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                short shortArray[] = {3, 4, 5};
                std::vector<short> ss(shortArray, shortArray + 3);
                int integerArray[] = {9, 8, 7, 6};
                std::vector<int> ii(integerArray, integerArray + 4);
                long longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<long> ll(longArray, longArray + 6);
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

                TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                boost::shared_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService);
                tip2 = serializationService.toObject<TestInnerPortable>(data);

                iTest::assertEqual(inner, *tip1);
                iTest::assertEqual(inner, *tip2);

                TestMainPortable main((byte)113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909, "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                boost::shared_ptr<TestMainPortable> tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService);
                tmp2 = serializationService.toObject<TestMainPortable>(data);
                iTest::assertEqual(main, *tmp1);
                iTest::assertEqual(main, *tmp2);
            }

            void ClientSerializationTest::testBasicFunctionalityWithLargeData() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                byte *byteArray = new byte[LARGE_ARRAY_SIZE];
                std::vector<byte> bb(byteArray, byteArray + LARGE_ARRAY_SIZE);
                char *charArray;
                charArray = new char[LARGE_ARRAY_SIZE];
                std::vector<char> cc(charArray, charArray + LARGE_ARRAY_SIZE);
                short *shortArray;
                shortArray = new short[LARGE_ARRAY_SIZE];
                std::vector<short> ss(shortArray, shortArray + LARGE_ARRAY_SIZE);
                int *integerArray;
                integerArray = new int[LARGE_ARRAY_SIZE];
                std::vector<int> ii(integerArray, integerArray + LARGE_ARRAY_SIZE);
                long *longArray;
                longArray = new long[LARGE_ARRAY_SIZE];
                std::vector<long> ll(longArray, longArray + LARGE_ARRAY_SIZE);
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

                TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                boost::shared_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService);
                tip2 = serializationService.toObject<TestInnerPortable>(data);

                iTest::assertEqual(inner, *tip1);
                iTest::assertEqual(inner, *tip2);

            }


            void ClientSerializationTest::testBasicFunctionalityWithDifferentVersions() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);
                serialization::pimpl::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);
                data = writeAndReadData(data, serializationService, serializationService2);
                boost::shared_ptr<int> returnedInt = serializationService.toObject<int>(data);
                iTest::assertEqual(x, *returnedInt);

                short f = 2;
                data = serializationService.toData<short>(&f);
                data = writeAndReadData(data, serializationService, serializationService2);
                boost::shared_ptr<short> temp = serializationService.toObject<short>(data);
                iTest::assertEqual(f, *temp);

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                boost::shared_ptr<TestNamedPortable> tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService2);
                tnp2 = serializationService2.toObject<TestNamedPortable>(data);

                iTest::assertEqual(np, *tnp1);
                iTest::assertEqual(np, *tnp2);

                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                short shortArray[] = {3, 4, 5};
                std::vector<short> ss(shortArray, shortArray + 3);
                int integerArray[] = {9, 8, 7, 6};
                std::vector<int> ii(integerArray, integerArray + 4);
                long longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<long> ll(longArray, longArray + 6);
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

                TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                boost::shared_ptr<TestInnerPortable> tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService2);
                tip2 = serializationService2.toObject<TestInnerPortable>(data);

                iTest::assertEqual(inner, *tip1);
                iTest::assertEqual(inner, *tip2);

                TestMainPortable main((byte)113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909, "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                boost::shared_ptr<TestMainPortable> tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable>(data);
                data = writeAndReadData(data, serializationService, serializationService2);
                tmp2 = serializationService2.toObject<TestMainPortable>(data);
                iTest::assertEqual(main, *tmp1);
                iTest::assertEqual(main, *tmp2);
            }


            void ClientSerializationTest::testTemplatedPortable_whenMultipleTypesAreUsed() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ParentTemplatedPortable<ChildTemplatedPortable1> portable(new ChildTemplatedPortable1("aaa", "bbb"));
                ss.toData<ParentTemplatedPortable<ChildTemplatedPortable1> >(&portable);
                ParentTemplatedPortable<ChildTemplatedPortable2> portable2(new ChildTemplatedPortable2("ccc"));

                bool expectedException = false;
                try {
                    ss.toData<ParentTemplatedPortable<ChildTemplatedPortable2> >(&portable2);
                } catch (exception::HazelcastSerializationException&) {
                    expectedException = true;
                }
                iTest::assertTrue(expectedException);

            }

            void ClientSerializationTest::testDataHash() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                std::string serializable = "key1";
                serialization::pimpl::Data data = serializationService.toData<std::string>(&serializable);
                serialization::pimpl::Data data2 = serializationService.toData<std::string>(&serializable);
                iTest::assertEqual(data.getPartitionHash(), data2.getPartitionHash());

            }

            void ClientSerializationTest::testPrimitives() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                byte by = 2;
                bool boolean = true;
                char c = 'c';
                short s = 4;
                int i = 2000;
                long l = 321324141;
                float f = 3.14;
                double d = 3.14334;
                std::string str = "Hello world";

                iTest::assertEqual(by,toDataAndBackToObject(serializationService, by));
                iTest::assertEqual(boolean,toDataAndBackToObject(serializationService, boolean));
                iTest::assertEqual(c,toDataAndBackToObject(serializationService, c));
                iTest::assertEqual(s,toDataAndBackToObject(serializationService, s));
                iTest::assertEqual(i,toDataAndBackToObject(serializationService, i));
                iTest::assertEqual(l,toDataAndBackToObject(serializationService, l));
                iTest::assertEqual(f,toDataAndBackToObject(serializationService, f));
                iTest::assertEqual(d,toDataAndBackToObject(serializationService, d));
                iTest::assertEqual(str,toDataAndBackToObject(serializationService, str));
            }

            void ClientSerializationTest::testPrimitiveArrays() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                short shortArray[] = {3, 4, 5};
                std::vector<short> ss(shortArray, shortArray + 3);
                int integerArray[] = {9, 8, 7, 6};
                std::vector<int> ii(integerArray, integerArray + 4);
                long longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<long> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);

                iTest::assertEqual(cc, toDataAndBackToObject(serializationService, cc));
                iTest::assertEqual(ss, toDataAndBackToObject(serializationService, ss));
                iTest::assertEqual(ii, toDataAndBackToObject(serializationService, ii));
                iTest::assertEqual(ll, toDataAndBackToObject(serializationService, ll));
                iTest::assertEqual(ff, toDataAndBackToObject(serializationService, ff));
                iTest::assertEqual(dd, toDataAndBackToObject(serializationService, dd));
            }


            void ClientSerializationTest::testWriteObjectWithPortable() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                TestNamedPortable *namedPortable = new TestNamedPortable("name", 2);
                ObjectCarryingPortable<TestNamedPortable> objectCarryingPortable(namedPortable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestNamedPortable> >(&objectCarryingPortable);
                boost::shared_ptr<ObjectCarryingPortable<TestNamedPortable> > ptr = ss.toObject<ObjectCarryingPortable<TestNamedPortable> >(data);
                iTest::assertEqual(objectCarryingPortable, *ptr);
            }

            void ClientSerializationTest::testWriteObjectWithIdentifiedDataSerializable() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                TestDataSerializable *testDataSerializable = new TestDataSerializable(2, 'c');
                ObjectCarryingPortable<TestDataSerializable> objectCarryingPortable(testDataSerializable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestDataSerializable> >(&objectCarryingPortable);
                boost::shared_ptr<ObjectCarryingPortable<TestDataSerializable> > ptr = ss.toObject<ObjectCarryingPortable<TestDataSerializable> >(data);
                iTest::assertEqual(objectCarryingPortable, *ptr);
            }

            void ClientSerializationTest::testWriteObjectWithCustomXSerializable() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                boost::shared_ptr<serialization::SerializerBase> serializer(new TestCustomSerializerX<TestCustomXSerializable>());

                ss.registerSerializer(serializer);

                TestCustomXSerializable* customXSerializable = new TestCustomXSerializable(131321);
                ObjectCarryingPortable<TestCustomXSerializable> objectCarryingPortable(customXSerializable);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestCustomXSerializable> >(&objectCarryingPortable);
                boost::shared_ptr<ObjectCarryingPortable<TestCustomXSerializable> > ptr = ss.toObject<ObjectCarryingPortable<TestCustomXSerializable> >(data);
                iTest::assertEqual(objectCarryingPortable, *ptr);


            }

            void ClientSerializationTest::testWriteObjectWithCustomPersonSerializable() {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                boost::shared_ptr<serialization::SerializerBase> serializer(new TestCustomPersonSerializer());

                ss.registerSerializer(serializer);

                TestCustomPerson* testCustomPerson = new TestCustomPerson("TestCustomPerson");

                ObjectCarryingPortable<TestCustomPerson> objectCarryingPortable(testCustomPerson);
                serialization::pimpl::Data data = ss.toData<ObjectCarryingPortable<TestCustomPerson> >(&objectCarryingPortable);
                boost::shared_ptr<ObjectCarryingPortable<TestCustomPerson> > ptr = ss.toObject<ObjectCarryingPortable<TestCustomPerson> >(data);
                iTest::assertEqual(objectCarryingPortable, *ptr);
            }


            void ClientSerializationTest::testNullData() {
                serialization::pimpl::Data data;
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                boost::shared_ptr<int> ptr = ss.toObject<int>(data);
                iTest::assertNull(ptr.get());
            }

            void ClientSerializationTest::testMorphingWithDifferentTypes_differentVersions() {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                TestNamedPortableV3 p2("portable-v2", 123);
                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV3>(&p2);

                data2 = writeAndReadData(data2, serializationService2, serializationService);
                boost::shared_ptr<TestNamedPortable> t1 = serializationService.toObject<TestNamedPortable>(data2);
                iTest::assertEqual(std::string("portable-v2"), t1->name);
                iTest::assertEqual(123, t1->k);


            }
        }
    }
}

