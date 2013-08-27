//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientSerializationTest.h"
#include "testUtil.h"
#include "TestCustomSerializerX.h"
#include "TestCustomXSerializable.h"
#include "TestCustomPersonSerializer.h"
#include "TestNamedPortableV2.h"
#include "TestRawDataPortable.h"
#include "TestInvalidReadPortable.h"
#include "TestInvalidWritePortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include <fstream>

using namespace iTest;

namespace hazelcast {
    namespace client {
        namespace test {

            ClientSerializationTest::ClientSerializationTest() {

            };

            void ClientSerializationTest::beforeClass() {
            };

            void ClientSerializationTest::afterClass() {
            };

            void ClientSerializationTest::beforeTest() {
            };

            void ClientSerializationTest::afterTest() {
            };

            void ClientSerializationTest::addTests() {
                addTest(&ClientSerializationTest::testCustomSerialization, "testCustomSerialization");
                addTest(&ClientSerializationTest::testRawData, "testRawData");
                addTest(&ClientSerializationTest::testIdentifiedDataSerializable, "testIdentifiedDataSerializable");
                addTest(&ClientSerializationTest::testRawDataWithoutRegistering, "testRawDataWithoutRegistering");
                addTest(&ClientSerializationTest::testInvalidWrite, "testInvalidWrite");
                addTest(&ClientSerializationTest::testInvalidRead, "testInvalidRead");
                addTest(&ClientSerializationTest::testDifferentVersions, "testDifferentVersions");
                addTest(&ClientSerializationTest::testCompression, "testCompression");
                addTest(&ClientSerializationTest::testSerializationViaFile, "testSerializationViaFile");
                addTest(&ClientSerializationTest::testBasicFunctionality, "testBasicFunctionality");
            };

            void ClientSerializationTest::testCustomSerialization() {
                serialization::SerializationService serializationService(1);

                TestCustomSerializerX<TestCustomXSerializable> serializer1;
                TestCustomPersonSerializer serializer2;
                serializationService.registerSerializer(&serializer1);
                serializationService.registerSerializer(&serializer2);

                TestCustomXSerializable a;
                a.id = 131321;
                serialization::Data data = serializationService.toData<TestCustomXSerializable>(&a);
                TestCustomXSerializable a2 = serializationService.toObject<TestCustomXSerializable>(data);
                assertEqual(a.id, a2.id);

                TestCustomPerson b;
                b.setName("TestCustomPerson");
                serialization::Data data1 = serializationService.toData<TestCustomPerson>(&b);
                TestCustomPerson b2 = serializationService.toObject<TestCustomPerson>(data1);
                assertEqual(std::string("TestCustomPerson"), b2.getName());
            };


            void ClientSerializationTest::testRawData() {
                serialization::SerializationService serializationService(1);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestDataSerializable ds(123, 's');
                TestNamedPortable np("named portable", 34567);
                TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);
                serialization::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addCharArrayField("c").addPortableField("p", 1, 3);
                serializationService.getSerializationContext().registerClassDefinition(builder.build());

                serialization::Data data = serializationService.toData<TestRawDataPortable>(&p);
                TestRawDataPortable x = serializationService.toObject<TestRawDataPortable>(data);
                assertEqual(p, x);
            };


            void ClientSerializationTest::testIdentifiedDataSerializable() {
                serialization::SerializationService serializationService(1);
                serialization::Data data;
                TestDataSerializable np(4, 'k');
                data = serializationService.toData<TestDataSerializable>(&np);

                TestDataSerializable tnp1;
                tnp1 = serializationService.toObject<TestDataSerializable>(data);

                assertEqual(np, tnp1);
                int x = 4;
                data = serializationService.toData<int>(&x);
                int y = serializationService.toObject<int>(data);
                assertEqual(x, y);
            };

            void ClientSerializationTest::testRawDataWithoutRegistering() {
                serialization::SerializationService serializationService(1);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestNamedPortable np("named portable", 34567);
                TestDataSerializable ds(123, 's');
                TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);

                serialization::Data data = serializationService.toData<TestRawDataPortable>(&p);
                TestRawDataPortable x = serializationService.toObject<TestRawDataPortable>(data);
                assertEqual(p, x);

            };


            void ClientSerializationTest::testInvalidWrite() {
                serialization::SerializationService serializationService(1);
                TestInvalidWritePortable p(2131, 123, "q4edfd");
                serialization::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addIntField("i").addUTFField("s");
                serializationService.getSerializationContext().registerClassDefinition(builder.build());
                serialization::Data data = serializationService.toData<TestInvalidWritePortable>(&p);
                TestInvalidWritePortable o = serializationService.toObject<TestInvalidWritePortable>(data);
            };

            void ClientSerializationTest::testInvalidRead() {
                serialization::SerializationService serializationService(1);
                TestInvalidReadPortable p(2131, 123, "q4edfd");
                serialization::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addIntField("i").addUTFField("s");
                serializationService.getSerializationContext().registerClassDefinition(builder.build());
                serialization::Data data = serializationService.toData<TestInvalidReadPortable>(&p);
                serializationService.toObject<TestInvalidReadPortable>(data);
            }

            void ClientSerializationTest::testDifferentVersions() {
                serialization::SerializationService serializationService(1);

                serialization::SerializationService serializationService2(2);

                TestNamedPortable p1("portable-v1", 111);
                serialization::Data data = serializationService.toData<TestNamedPortable>(&p1);

                TestNamedPortableV2 p2("portable-v2", 123);
                serialization::Data data2 = serializationService2.toData<TestNamedPortableV2>(&p2);

                TestNamedPortableV2 t2 = serializationService2.toObject<TestNamedPortableV2>(data);
                assertEqual(std::string("portable-v1"), t2.name);
                assertEqual(111, t2.k);
                assertEqual(0, t2.v);

                TestNamedPortable t1 = serializationService.toObject<TestNamedPortable>(data2);
                assertEqual(std::string("portable-v2"), t1.name);
                assertEqual(123 * 10, t1.k);

            };

            void ClientSerializationTest::testCompression() {
                serialization::SerializationService serializationService1(1);
                TestMainPortable mainPortable = getTestMainPortable();

                serialization::Data data = serializationService1.toData<TestMainPortable>(&mainPortable);

                serialization::DataOutput out;
                data.writeData(out);

                std::auto_ptr< vector < byte> > xxx = out.toByteArray();

                serialization::SerializationService serializationService2(1);

                serialization::DataInput dataInput(*(xxx.get()));
                serialization::ObjectDataInput objectDataInput(dataInput, serializationService2.getSerializationContext());
                serialization::Data newData;
                newData.readData(objectDataInput);
                TestMainPortable returnedPortable = serializationService2.toObject<TestMainPortable >(newData);
                assertEqual(mainPortable, returnedPortable);
            };


            int ClientSerializationTest::write() {
                serialization::SerializationService serializationService(1);
                TestMainPortable mainPortable = getTestMainPortable();
                serialization::Data data = serializationService.toData<TestMainPortable>(&mainPortable);
                serialization::DataOutput out;
                data.writeData(out);
                std::vector<byte>& outBuffer = *(out.toByteArray().get());
                int size = outBuffer.size();
                ofstream outfile;
                outfile.open("./text.txt", std::ios_base::out);
                for (int i = 0; i < size; i++)
                    outfile.put(outBuffer[i]);

                outfile.close();
                return size;

            }

            void ClientSerializationTest::read(int size) {

                serialization::SerializationService serializationService(1);

                std::ifstream is;
                is.open("./text.txt", std::ios::binary);
                char bytes[size];
                is.read(bytes, size);
                is.close();

                byte *tempPtr = (byte *) bytes;
                std::vector<byte> buffer(tempPtr, tempPtr + size);

                serialization::DataInput dataInput(buffer);
                serialization::ObjectDataInput objectDataInput(dataInput, serializationService.getSerializationContext());

                serialization::Data data;

                data.readData(objectDataInput);

                TestMainPortable tmp1 = serializationService.toObject<TestMainPortable>(data);

                TestMainPortable mainPortable = getTestMainPortable();
                assertEqual(mainPortable, tmp1);
            };

            void ClientSerializationTest::testSerializationViaFile() {
                int size = write();
                read(size);
            }


            void ClientSerializationTest::testBasicFunctionality() {
                serialization::SerializationService serializationService(1);
                serialization::SerializationService serializationService2(2);
                serialization::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);
                assertEqual(x, serializationService.toObject<int>(data));

                short f = 3.2;
                data = serializationService.toData<short>(&f);
                assertEqual(f, serializationService.toObject<short>(data));

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                TestNamedPortable tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable >(data);
                tnp2 = serializationService2.toObject<TestNamedPortable >(data);

                assertEqual(np, tnp1);
                assertEqual(np, tnp2);

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
                    string x = "named-portable-";
                    x.push_back('0' + i);
                    portableArray[i].name = x;
                    portableArray[i].k = i;
                }
                std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

                TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);

                data = serializationService.toData<TestInnerPortable>(&inner);

                TestInnerPortable tip1, tip2;
                tip1 = serializationService.toObject<TestInnerPortable >(data);
                tip2 = serializationService2.toObject<TestInnerPortable >(data);

                assertEqual(inner, tip1);
                assertEqual(inner, tip2);

                TestMainPortable main((byte) 113, true, 'x', -500, 56789, -50992225, 900.5678,
                        -897543.3678909, "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                TestMainPortable tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable >(data);
                tmp2 = serializationService2.toObject<TestMainPortable >(data);
                assertEqual(main, tmp1);
                assertEqual(main, tmp2);
            };
        }
    }
}