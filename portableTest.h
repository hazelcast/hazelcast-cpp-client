#ifndef PORTABLE_TEST_UTIL
#define PORTABLE_TEST_UTIL

#include "testUtil.h"
#include "TestNamedPortableV2.h"
#include "TestRawDataPortable.h"
#include "TestInvalidReadPortable.h"
#include "TestInvalidWritePortable.h"
#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/protocol/HazelcastServerError.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/connection/Socket.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <fstream>


void testBinaryClient() {
    hazelcast::client::Address address(SERVER_ADDRESS, SERVER_PORT);
    hazelcast::client::connection::Socket socket(address);
    SerializationService service(0);
    ClassDefinitionBuilder cd(-3, 3);
    ClassDefinition *ptr = cd.addUTFField("uuid").addUTFField("ownerUuid").build();
    service.getSerializationContext()->registerClassDefinition(ptr);

    hazelcast::client::protocol::Credentials credentials("sancar", "dev-pass");
    hazelcast::client::protocol::AuthenticationRequest ar(credentials);
    Data data;
    service.toData(ar, data);
    socket.connect();
    socket.send("CB1", 3);
    OutputSocketStream output(socket);
    data.writeData(output);

    data.setSerializationContext(service.getSerializationContext());
    InputSocketStream input(socket);
    data.readData(input);

    if (data.isServerError()) {
        hazelcast::client::protocol::HazelcastServerError error;
        service.toObject(data, error);
        std::cout << "type:" << error.type << std::endl;
        std::cout << "message:" << error.message << std::endl;
        std::cout << "details:" << error.details << std::endl;
    } else {
        hazelcast::client::protocol::Principal principal;
        service.toObject(data, principal);
        std::cout << principal.uuid << std::endl;
        std::cout << principal.ownerUuid << std::endl;

    }
    service.toData(ar, data);
    data.writeData(output);
    data.readData(input);
    if (data.isServerError()) {
        hazelcast::client::protocol::HazelcastServerError error;
        service.toObject(data, error);
        std::cout << "type:" << error.type << std::endl;
        std::cout << "message:" << error.message << std::endl;
        std::cout << "details:" << error.details << std::endl;
    } else {
        hazelcast::client::protocol::Principal principal;
        service.toObject(data, principal);
        std::cout << principal.uuid << std::endl;
        std::cout << principal.ownerUuid << std::endl;

    }
}

void testRawData() {
    serialization::SerializationService serializationService(1);
    char charA[] = "test chars";
    std::vector<char> chars(charA, charA + 10);
    std::vector<byte> bytes;
    bytes.resize(5, 0);
    TestDataSerializable ds(123, 's');
    TestNamedPortable np("named portable", 34567);
    TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);
    ClassDefinitionBuilder builder(getFactoryId(p), getClassId(p));
    builder.addLongField("l").addCharArrayField("c").addPortableField("p", 1, 3);
    serializationService.getSerializationContext()->registerClassDefinition(builder.build());

    Data data;
    serializationService.toData(p, data);
    TestRawDataPortable x;
    serializationService.toObject(data, x);
    assert(p == x);

}

void testIdentifiedDataSerializable() {
    SerializationService serializationService(1);
    Data data;
    TestDataSerializable np(4, 'k');
    serializationService.toData(np, data);

    TestDataSerializable tnp1;
    serializationService.toObject(data, tnp1);

    assert(np == tnp1);
    int x = 4;
    serializationService.toData(x, data);
    int y;
    serializationService.toObject(data, y);
    assert(x == y);
};

void testRawDataWithoutRegistering() {
    serialization::SerializationService serializationService(1);
    char charA[] = "test chars";
    std::vector<char> chars(charA, charA + 10);
    std::vector<byte> bytes;
    bytes.resize(5, 0);
    TestNamedPortable np("named portable", 34567);
    TestDataSerializable ds(123, 's');
    TestRawDataPortable p(123213, chars, np, 22, "Testing raw portable", ds);

    Data data;
    serializationService.toData(p, data);
    TestRawDataPortable x;
    serializationService.toObject<TestRawDataPortable>(data, x);
    assert(p == x);

}

void testRawDataInvalidWrite() {
    SerializationService serializationService(1);
    TestInvalidWritePortable p(2131, 123, "q4edfd");
    ClassDefinitionBuilder builder(getFactoryId(p), getClassId(p));
    builder.addLongField("l").addIntField("i").addUTFField("s");
    serializationService.getSerializationContext()->registerClassDefinition(builder.build());
    try{
        Data data;
        serializationService.toData(p, data);
        TestInvalidWritePortable o;
        serializationService.toObject(data, o);

    }  catch (HazelcastException& exception) {
        std::cout << "Expected exception " << exception.what() << std::endl;
    }
}

void testRawDataInvalidRead() {
    SerializationService serializationService(1);
    TestInvalidReadPortable p(2131, 123, "q4edfd");
    ClassDefinitionBuilder builder(getFactoryId(p), getClassId(p));
    builder.addLongField("l").addIntField("i").addUTFField("s");
    serializationService.getSerializationContext()->registerClassDefinition(builder.build());
    try{
        Data data;
        serializationService.toData(p, data);
        TestInvalidReadPortable tip;
        serializationService.toObject(data, tip);

    }  catch (HazelcastException& exception) {
        std::cout << "Expected exception " << exception.what() << std::endl;
    }
}

void testDifferentVersions() {
    serialization::SerializationService serializationService(1);

    serialization::SerializationService serializationService2(2);

    TestNamedPortable p1("portable-v1", 111);
    Data data;
    serializationService.toData(p1, data);

    TestNamedPortableV2 p2("portable-v2", 123);
    Data data2;
    serializationService2.toData(p2, data2);

    TestNamedPortableV2 t2;
    serializationService2.toObject(data, t2);
    assert(t2.name.compare("portable-v1") == 0);
    assert(t2.k == 111);
    assert(t2.v == 0);

    TestNamedPortable t1;
    serializationService.toObject(data2, t1);
    assert(t1.name.compare("portable-v2") == 0);
    assert(t1.k == 123 * 10);

};

void testCompression() {
    serialization::SerializationService serializationService1(1);
    TestMainPortable mainPortable = getTestMainPortable();

    Data data;
    serializationService1.toData(mainPortable, data);

    BufferedDataOutput out;
    data.writeData(out);

    std::auto_ptr< vector<byte> > xxx = out.toByteArray();

    serialization::SerializationService serializationService2(1);
    serialization::BufferedDataInput dataInput(*(xxx.get()));
    Data newData;
    newData.setSerializationContext(serializationService2.getSerializationContext());
    newData.readData(dataInput);
    TestMainPortable returnedPortable;
    serializationService2.toObject<TestMainPortable >(newData, returnedPortable);
    assert(returnedPortable == mainPortable);
};


int write() {
    serialization::SerializationService serializationService(1);
    TestMainPortable mainPortable = getTestMainPortable();
    Data data;
    serializationService.toData(mainPortable, data);
    BufferedDataOutput out;
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

void read(int size) {

    serialization::SerializationService serializationService(1);

    std::ifstream is;
    is.open("./text.txt", std::ios::binary);
    char bytes[size];
    is.read(bytes, size);
    is.close();

    byte *tempPtr = (byte *) bytes;
    std::vector<byte> buffer(tempPtr, tempPtr + size);
    serialization::BufferedDataInput dataInput(buffer);

    serialization::Data data;
    data.setSerializationContext(serializationService.getSerializationContext());
    data.readData(dataInput);

    TestMainPortable tmp1;
    serializationService.toObject(data, tmp1);

    TestMainPortable mainPortable = getTestMainPortable();
    assert(mainPortable == tmp1);
};

void testSerializationViaFile() {
    int size = write();
    read(size);
}


void testSerialization() {
    serialization::SerializationService serializationService(1);
    serialization::SerializationService serializationService2(2);
    serialization::Data data;

    int x = 3;
    serializationService.toData(x, data);
    int rx;
    serializationService.toObject(data, rx);
    assert(x == rx);

    short f = 3.2;
    serializationService.toData(f, data);
    short rf;
    serializationService.toObject(data, rf);
    assert(f == rf);

    TestNamedPortable np("name", 5);
    serializationService.toData(np, data);

    TestNamedPortable tnp1, tnp2;
    serializationService.toObject(data, tnp1);
    serializationService2.toObject(data, tnp2);

    assert(np == tnp1);
    assert(np == tnp2);

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

    serializationService.toData(inner, data);

    TestInnerPortable tip1, tip2;
    serializationService.toObject(data, tip1);
    serializationService2.toObject(data, tip2);

    assert(inner == tip1);
    assert(inner == tip2);

    TestMainPortable main((byte) 113, true, 'x', -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);
    serializationService.toData(main, data);

    TestMainPortable tmp1, tmp2;
    serializationService.toObject(data, tmp1);
    serializationService2.toObject(data, tmp2);
    assert(main == tmp1);
    assert(main == tmp2);


};

#endif

