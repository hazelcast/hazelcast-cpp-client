//
// server.cpp

#include "TestPortableFactory.h"
#include "TestNamedPortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"
#include "SimpleMapTest.h"

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/Data.h" 
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"

#include <boost/thread.hpp>

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <set>
#include <memory>
#include <cstdio>
using namespace hazelcast::client;

void testSpeed();
void testCompression();
void testSerialization();
void testSerializationViaFile();
void testMapOperations();
void testMapLocksInParallel();
void testMapLocksInSequential();
void write();
void read();
TestMainPortable getTestMainPortable();

int main(int argc, char** argv) {
    try {
//        testCompression();
        testSerialization();
        testSerializationViaFile();
//        testMapOperations();
        //    testMapLocksInSequential();
        //    testMapLocksInParalled();
//        testSpeed();
        std::cout << "Test are completed successfully" << std::endl;
    } catch (const char* s) {
        printf("%s", s);
    }
    return 0;
};

void testCompression() {
    TestPortableFactory tpf1;
    serialization::SerializationService serializationService1(1, &tpf1);
    TestMainPortable mainPortable = getTestMainPortable();

    Data data = serializationService1.toData(mainPortable);

    DataOutput* out = serializationService1.pop();
    data.writeData(*out);

    vector<byte> xxx = out->toByteArray();

    serialization::SerializationService serializationService2(1, &tpf1);
    serialization::DataInput dataInput(xxx, &serializationService2);
    Data newData;
    newData.readData(dataInput);
    TestMainPortable returnedPortable = serializationService2.toObject<TestMainPortable > (newData);
    assert(returnedPortable == mainPortable);
};

void testMapOperations() {
    TestPortableFactory tpf1;
    ClientConfig clientConfig;
    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
    clientConfig.setAddress("192.168.2.7:5701");
    clientConfig.setPortableFactory(&tpf1);

    try {

        auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);
        IMap<int, TestMainPortable> imap = hazelcastClient->getMap<int, TestMainPortable > ("kamil");
        TestMainPortable mainPortable = getTestMainPortable();
        for (int i = 0; i < 100; i++) {
            TestMainPortable x = mainPortable;
            x.i = i * 10;
            x.p.ii.push_back(i * 100);
            imap.put(i, x);
        }

        for (int i = 0; i < 100; i++) {
            TestMainPortable x = imap.get(i);
            assert(x.i == i * 10);
            assert(x.p.ii.at(x.p.ii.size() - 1) == i * 100);
        }

        assert(imap.containsKey(2) == true);
        assert(imap.containsKey(120) == false);

        TestMainPortable temp = imap.get(5);
        assert(imap.containsValue(temp) == true);
        temp.i = 2;
        assert(imap.containsValue(temp) == false);

        set<int> keys = imap.keySet();
        int i = 0;
        for (set<int>::iterator it = keys.begin(); it != keys.end(); it++) {
            assert(*it == i++);
        }
        vector<TestMainPortable> values = imap.values();
        i = 0;
        for (vector<TestMainPortable>::iterator it = values.begin(); it != values.end(); it++) {
            assert(it->i == i * 10);
            assert(it->p.ii.at(it->p.ii.size() - 1) == i * 100);
            i++;
        }

        int myints[] = {0, 2, 4, 6};
        std::set<int> keySet(myints, myints + sizeof (myints) / sizeof (int));
        std::map<int, TestMainPortable> stdMap = imap.getAll(keySet);
        for (int i = 0; i < 8; i += 2) {
            TestMainPortable x = stdMap[i];
            assert(x.i == i * 10);
            assert(x.p.ii.at(x.p.ii.size() - 1) == i * 100);
        }

        std::pair<int, TestMainPortable> entry = imap.getEntry(3);
        assert(entry.first == 3);
        assert(entry.second.i = 30);
        assert(entry.second.p.ii.at(entry.second.p.ii.size() - 1) == 300);

        std::vector< std::pair<int, TestMainPortable> > entrySet = imap.entrySet();
        std::vector< std::pair<int, TestMainPortable> >::iterator it;
        i = 0;
        for (it = entrySet.begin(); it != entrySet.end(); it++) {
            assert(it->first == i);
            assert(it->second.i == i * 10);
            assert(it->second.p.ii.at(it->second.p.ii.size() - 1) == i * 100);
            i++;
        }

        imap.remove(9);
        assert(imap.containsKey(9) == false);

        assert(imap.tryRemove(8, 1000) == true);
        assert(imap.containsKey(8) == false);
        //        imap.flush(); TODO
        assert(imap.evict(8) == false);
        assert(imap.evict(7) == true);
        assert(imap.containsKey(7) == false);


        assert(imap.tryPut(7, temp, 1000) == true);
        assert(imap.containsKey(7) == true);

        imap.put(8, temp, 0);
        assert(imap.containsKey(8) == true);

        imap.putTransient(9, temp, 0);
        assert(imap.containsKey(9) == true);

        TestMainPortable value6 = imap.get(6);
        assert(imap.replace(6, value6, temp) == true);
        assert(imap.get(6) == temp);
        assert(imap.replace(2, value6, temp) == false);
        assert(imap.get(2) != temp);

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
};

void testMapLocksInParallel() {

    try {

        boost::thread t1(testMapLocksInSequential);
        boost::thread t2(testMapLocksInSequential);

        t1.join();
        t2.join();

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
};

void testMapLocksInSequential() {
    ClientConfig clientConfig;
    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
    clientConfig.setAddress("192.168.2.7:5701");

    try {
        auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);

        IMap<int, int> imap = hazelcastClient->getMap<int, int > ("testLockMap");
        boost::hash<boost::thread::id> h;
        long currentId = h(boost::this_thread::get_id());

        imap.lock(0);
        std::cout << "pre " << currentId << endl;
        imap.unlock(0);

        imap.lock(1);
        std::cout << "critical " << currentId << endl;
        imap.unlock(1);

        imap.lock(2);
        std::cout << "out " << currentId << endl;
        imap.unlock(2);

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    //        imap.lock(1);
    //        imap.unlock(5);
    //        imap.isLocked(4);
    //        imap.forceunlock(3);
    //        imap.tryLock(3,1000);
};

void write() {
    TestPortableFactory tpf1;
    serialization::SerializationService serializationService(1, &tpf1);
    TestMainPortable mainPortable = getTestMainPortable();
    Data data = serializationService.toData(mainPortable);
    DataOutput* out = serializationService.pop();
    data.writeData(*out);
    std::vector<byte> outBuffer = out->toByteArray();

    ofstream outfile;
    outfile.open("./text.txt", std::ios_base::out);
    for (int i = 0; i < outBuffer.size(); i++)
        outfile.put(outBuffer[i]);

    serializationService.push(out);
    outfile.close();

}

void read() {

    TestPortableFactory tpf1;
    serialization::SerializationService serializationService(1, &tpf1);

    std::ifstream is;
    is.open("./text.txt", std::ios::binary);
    char bytes[673];
    is.read(bytes, 673);
    is.close();

    byte* tempPtr = (byte*) bytes;
    std::vector<byte> buffer(tempPtr, tempPtr + 673);
    serialization::DataInput dataInput(buffer, &serializationService);

    serialization::Data data;
    data.readData(dataInput);

    TestMainPortable tmp1;
    tmp1 = serializationService.toObject<TestMainPortable > (data);

    TestMainPortable mainPortable = getTestMainPortable();
    assert(mainPortable == tmp1);
};

void testSerializationViaFile() {
    write();
    read();
}

void testSerialization() {
    TestPortableFactory tpf1, tpf2;
    serialization::SerializationService serializationService(1, &tpf1);
    serialization::SerializationService serializationService2(2, &tpf2);
    serialization::Data data;

    int x = 3;
    data = serializationService.toData(x);
    assert(x == serializationService.toObject<int>(data));

    short f = 3.2;
    data = serializationService.toData(f);
    assert(f == serializationService.toObject<short>(data));

    TestNamedPortable np("name", 5);
    data = serializationService.toData(np);

    TestNamedPortable tnp1, tnp2;
    tnp1 = serializationService.toObject<TestNamedPortable > (data);
    tnp2 = serializationService2.toObject<TestNamedPortable > (data);

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
        portableArray[i] = TestNamedPortable(x, i);
    }
    std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

    TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);

    data = serializationService.toData(inner);

    TestInnerPortable tip1, tip2;
    tip1 = serializationService.toObject<TestInnerPortable > (data);
    tip2 = serializationService2.toObject<TestInnerPortable > (data);


    assert(inner == tip1);
    assert(inner == tip2);


    TestMainPortable main((byte) 113, true, 'x', (short) - 500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);
    data = serializationService.toData(main);

    TestMainPortable tmp1, tmp2;
    tmp1 = serializationService.toObject<TestMainPortable > (data);
    tmp2 = serializationService2.toObject<TestMainPortable > (data);
    assert(main == tmp1);
    assert(main == tmp2);


};

TestMainPortable getTestMainPortable() {
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
        portableArray[i] = TestNamedPortable(x, i);
    }
    std::vector<TestNamedPortable> nn(portableArray, portableArray + 5);

    TestInnerPortable inner(bb, cc, ss, ii, ll, ff, dd, nn);
    TestMainPortable main((byte) 113, true, 'x', (short) - 500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);
    return main;
};

void testSpeed() {
    SimpleMapTest s;
    s.run();
};
