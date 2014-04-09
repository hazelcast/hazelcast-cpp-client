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
#include "serialization/testUtil.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionBuilder.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "serialization/ClientSerializationTest.h"
#include <fstream>

namespace hazelcast {
    namespace client {
        namespace test {

            ClientSerializationTest::ClientSerializationTest()
            : iTest::iTestFixture<ClientSerializationTest>("ClientSerializationTest") {

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
                serialization::pimpl::SerializationService serializationService(1);

                boost::shared_ptr<serialization::SerializerBase> serializer1(new TestCustomSerializerX<TestCustomXSerializable>());
                boost::shared_ptr<serialization::SerializerBase> serializer2(new TestCustomPersonSerializer());

                serializationService.registerSerializer(serializer1);
                serializationService.registerSerializer(serializer2);

                TestCustomXSerializable a;
                a.id = 131321;
                serialization::pimpl::Data data = serializationService.toData<TestCustomXSerializable>(&a);
                boost::shared_ptr<TestCustomXSerializable> a2 = serializationService.toObject<TestCustomXSerializable>(data);
                iTest::assertEqual(a.id, a2->id);

                TestCustomPerson b;
                b.setName("TestCustomPerson");
                serialization::pimpl::Data data1 = serializationService.toData<TestCustomPerson>(&b);
                boost::shared_ptr<TestCustomPerson> b2 = serializationService.toObject<TestCustomPerson>(data1);
                iTest::assertEqual(std::string("TestCustomPerson"), b2->getName());
            };


            void ClientSerializationTest::testRawData() {
                serialization::pimpl::SerializationService serializationService(1);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestDataSerializable ds(123, 's');
                TestNamedPortable np("named portable", 34567);
                TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);
                serialization::pimpl::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addCharArrayField("c").addPortableField("p", 1, 3);
                boost::shared_ptr<serialization::pimpl::ClassDefinition> cd(builder.build());
                serializationService.getSerializationContext().registerClassDefinition(cd);

                serialization::pimpl::Data data = serializationService.toData<TestRawDataPortable>(&p);
                boost::shared_ptr<TestRawDataPortable> x = serializationService.toObject<TestRawDataPortable>(data);
                iTest::assertEqual(p, *x);
            };


            void ClientSerializationTest::testIdentifiedDataSerializable() {
                serialization::pimpl::SerializationService serializationService(1);
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
            };

            void ClientSerializationTest::testRawDataWithoutRegistering() {
                serialization::pimpl::SerializationService serializationService(1);
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

            };


            void ClientSerializationTest::testInvalidWrite() {
                serialization::pimpl::SerializationService serializationService(1);
                TestInvalidWritePortable p(2131, 123, "q4edfd");
                serialization::pimpl::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addIntField("i").addUTFField("s");
                boost::shared_ptr<serialization::pimpl::ClassDefinition> cd(builder.build());
                serializationService.getSerializationContext().registerClassDefinition(cd);
                try{
                    serializationService.toData<TestInvalidWritePortable>(&p);
                    iTest::assertTrue(false, "toData should trow exception");
                }catch(exception::IOException &){

                }
            };

            void ClientSerializationTest::testInvalidRead() {
                serialization::pimpl::SerializationService serializationService(1);
                TestInvalidReadPortable p(2131, 123, "q4edfd");
                serialization::pimpl::ClassDefinitionBuilder builder(p.getFactoryId(), p.getClassId());
                builder.addLongField("l").addIntField("i").addUTFField("s");
                boost::shared_ptr<serialization::pimpl::ClassDefinition> cd(builder.build());
                serializationService.getSerializationContext().registerClassDefinition(cd);
                serialization::pimpl::Data data = serializationService.toData<TestInvalidReadPortable>(&p);
                try{
                    serializationService.toObject<TestInvalidReadPortable>(data);
                    iTest::assertTrue(false, "toObject should trow exception");
                }catch(exception::IOException &){

                }
            }

            void ClientSerializationTest::testDifferentVersions() {
                serialization::pimpl::SerializationService serializationService(1);

                serialization::pimpl::SerializationService serializationService2(2);

                TestNamedPortable p1("portable-v1", 111);
                serialization::pimpl::Data data = serializationService.toData<TestNamedPortable>(&p1);

                TestNamedPortableV2 p2("portable-v2", 123);
                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV2>(&p2);

                boost::shared_ptr<TestNamedPortableV2> t2 = serializationService2.toObject<TestNamedPortableV2>(data);
                iTest::assertEqual(std::string("portable-v1"), t2->name);
                iTest::assertEqual(111, t2->k);
                iTest::assertEqual(0, t2->v);

                boost::shared_ptr<TestNamedPortable> t1 = serializationService.toObject<TestNamedPortable>(data2);
                iTest::assertEqual(std::string("portable-v2"), t1->name);
                iTest::assertEqual(123 * 10, t1->k);

            };

            void ClientSerializationTest::testCompression() {
                serialization::pimpl::SerializationService serializationService1(1);
                TestMainPortable mainPortable = getTestMainPortable();

                serialization::pimpl::Data data = serializationService1.toData<TestMainPortable>(&mainPortable);

                serialization::pimpl::DataOutput dataOutput;
                serialization::ObjectDataOutput objectDataOutput(dataOutput, serializationService1.getSerializationContext());
                data.writeData(objectDataOutput);

                std::auto_ptr< std::vector < byte> > xxx = objectDataOutput.toByteArray();

                serialization::pimpl::SerializationService serializationService2(1);

                serialization::pimpl::DataInput dataInput(*(xxx.get()));
                serialization::ObjectDataInput objectDataInput(dataInput, serializationService2.getSerializationContext());
                serialization::pimpl::Data newData;
                newData.readData(objectDataInput);
                boost::shared_ptr<TestMainPortable> returnedPortable = serializationService2.toObject<TestMainPortable >(newData);
                iTest::assertEqual(mainPortable, *returnedPortable);
            };


            int ClientSerializationTest::write() {
                serialization::pimpl::SerializationService serializationService(1);
                TestMainPortable mainPortable = getTestMainPortable();
                serialization::pimpl::Data data = serializationService.toData<TestMainPortable>(&mainPortable);
                serialization::pimpl::DataOutput dataOutput;
                serialization::ObjectDataOutput objectDataOutput(dataOutput, serializationService.getSerializationContext());
                data.writeData(objectDataOutput);
                std::auto_ptr<std::vector<byte> > output = objectDataOutput.toByteArray();
                int size = output->size();
                std::ofstream outfile;
                outfile.open("./text.txt", std::ios_base::out);
                for (int i = 0; i < size; i++)
                    outfile.put((*output)[i]);

                outfile.close();
                return size;

            }

            void ClientSerializationTest::read(int size) {

                serialization::pimpl::SerializationService serializationService(1);

                std::ifstream is;
                is.open("./text.txt", std::ios::binary);
                char *bytes = new char [size];
                is.read(bytes, size);
                is.close();

                byte *tempPtr = (byte *) bytes;
                std::vector<byte> buffer(tempPtr, tempPtr + size);
                delete [] bytes;
                serialization::pimpl::DataInput dataInput(buffer);
                serialization::ObjectDataInput objectDataInput(dataInput, serializationService.getSerializationContext());

                serialization::pimpl::Data data;

                data.readData(objectDataInput);

                boost::shared_ptr<TestMainPortable> tmp1 = serializationService.toObject<TestMainPortable>(data);

                TestMainPortable mainPortable = getTestMainPortable();
                iTest::assertEqual(mainPortable, *tmp1);
            };

            void ClientSerializationTest::testSerializationViaFile() {
                int size = write();
                read(size);
            }


            void ClientSerializationTest::testBasicFunctionality() {
                serialization::pimpl::SerializationService serializationService(1);
                serialization::pimpl::SerializationService serializationService2(2);
                serialization::pimpl::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);
                boost::shared_ptr<int> returnedInt = serializationService.toObject<int>(data);
                iTest::assertEqual(x, *returnedInt);

                short f = 2;
                data = serializationService.toData<short>(&f);
                boost::shared_ptr<short> temp = serializationService.toObject<short>(data);
                iTest::assertEqual(f, *temp);

                TestNamedPortable np("name", 5);
                data = serializationService.toData<TestNamedPortable>(&np);

                boost::shared_ptr<TestNamedPortable> tnp1, tnp2;
                tnp1 = serializationService.toObject<TestNamedPortable >(data);
                tnp2 = serializationService2.toObject<TestNamedPortable >(data);

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
                tip1 = serializationService.toObject<TestInnerPortable >(data);
                tip2 = serializationService2.toObject<TestInnerPortable >(data);

                iTest::assertEqual(inner, *tip1);
                iTest::assertEqual(inner, *tip2);

                TestMainPortable main((byte) 113, true, 'x', -500, 56789, -50992225, 900.5678,
                        -897543.3678909, "this is main portable object created for testing!", inner);
                data = serializationService.toData<TestMainPortable>(&main);

                boost::shared_ptr<TestMainPortable> tmp1, tmp2;
                tmp1 = serializationService.toObject<TestMainPortable >(data);
                tmp2 = serializationService2.toObject<TestMainPortable >(data);
                iTest::assertEqual(main, *tmp1);
                iTest::assertEqual(main, *tmp2);
            };
        }
    }
}

