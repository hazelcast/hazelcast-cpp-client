//
// server.cpp

#include "TestPortableFactory.h"
#include "TestNamedPortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/IMap.cpp"


#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <set>
#include <memory>
using namespace hazelcast::client;

void write();
void read();
void client();
TestMainPortable getTestMainPortable();

int main(int argc, char** argv){
//    write();
//    read();
    client();
    return 0;
};

void client(){
    TestPortableFactory tpf1;
    ClientConfig clientConfig;
    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
    clientConfig.setAddress("192.168.2.2:5701");
    clientConfig.setPortableFactory(&tpf1);
    
    try{
        auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);
        IMap<int,TestMainPortable> imap = hazelcastClient->getMap<int,TestMainPortable>("ali");
        std::cout << imap.getName() << std::endl;
        TestMainPortable mainPortable = getTestMainPortable();
        for(int i = 0 ; i < 100 ; i++){
            TestMainPortable x = mainPortable;
            x.i = i * 10;
            x.p.ii.push_back(i*100); 
            imap.put(i,x);
        }
        
        for(int i = 0 ; i < 100 ; i++){
            TestMainPortable x = imap.get(i);
             assert(x.i == i*10);
             assert(x.p.ii.at(x.p.ii.size() -1 ) == i*100);
        }
        assert(imap.containsKey(2) == true);
        assert(imap.containsKey(120) == false);
        
        TestMainPortable temp = imap.get(5);
        assert(imap.containsValue(temp) == true);
        temp.i = 2;
        assert(imap.containsValue(temp) == false);
        
        
        int myints[] = {0,2,4,6};
        std::set<int> keySet (myints, myints + sizeof(myints) / sizeof(int) );
        
        std::map<int,TestMainPortable> stdMap = imap.getAll(keySet);
        for(int i = 0 ; i < 8 ; i+=2 ){
            TestMainPortable x = stdMap[i];
            assert(x.i == i*10);
            assert(x.p.ii.at(x.p.ii.size()-1) == i*100);
        }
        imap.remove(9);
        assert(imap.containsKey(9) == false);
        assert(imap.tryRemove(8,1000) == true);
        assert(imap.containsKey(8) == false);
        
//        imap.flush(); TODO
        assert(imap.evict(8) == false);
        assert(imap.evict(7) == true);
        assert(imap.containsKey(7) == false);
        
//        TestMainPortable z = imap.get(0);
//        std::cout << "put 1 is successful"  << imap.tryPut(1,z,1000) << std::endl;
//        imap.put(2,z,2000);
//        imap.putTransient(3,z,2000);
//        
//        std::cout << "replaceIfSame 3 is successful"  << imap.replace(3,z,z) << std::endl;
//        std::cout << "replaceIfSame 4 is successful"  << imap.replace(6,z,z) << std::endl;
//        
//        
//        set<int> keys = imap.keySet();
//        vector<TestMainPortable> value = imap.values();
//        
//        std::pair<int,TestMainPortable> entry = imap.getEntry(3);
//        std::vector< std::pair<int,TestMainPortable> > entrySet = imap.entrySet();
//        
//        imap.lock(1);
//        imap.unlock(5);
//        imap.isLocked(4);
//        imap.forceunlock(3);
//        imap.tryLock(3,1000);
        
//        std::cout << "Press a key to end" << std::endl;
//        std::string x;
//        std::cin >> x;
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
};

void read(){
    
    TestPortableFactory tpf1;
    serialization::SerializationService serializationService(1, &tpf1);

    std::ifstream is;
    is.open ("/Users/msk/Desktop/text.txt", std::ios::binary );
    char bytes[673];
    is.read(bytes,673);
    is.close();
    
    byte* tempPtr = (byte*)bytes;
    std::vector<byte> buffer(tempPtr,tempPtr + 673);
    serialization::DataInput dataInput(buffer,&serializationService);
    
    serialization::Data data;
    data.readData(dataInput);
    
    TestMainPortable tmp1;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
    
    TestMainPortable mainPortable = getTestMainPortable();
    assert(mainPortable == tmp1);
};

void write(){
    TestPortableFactory tpf1,tpf2;
    serialization::SerializationService serializationService(1, &tpf1);
    serialization::SerializationService serializationService2(2, &tpf2);
    serialization::Data data;
    
    int x = 3;
    data = serializationService.toData(x);
    assert( x == serializationService.toObject<int>(data) );
    
    short f = 3.2;
    data = serializationService.toData(f);
    assert( f == serializationService.toObject<short>(data) );

    TestNamedPortable np("name",5);
    data = serializationService.toData(np);
    
    TestNamedPortable tnp1,tnp2;
    tnp1 = serializationService.toObject<TestNamedPortable>(data);
    tnp2 = serializationService2.toObject<TestNamedPortable>(data);
    
    assert(np == tnp1);
    assert(np == tnp2);
    
    byte byteArray[]= {0, 1, 2};
    std::vector<byte> bb(byteArray,byteArray+3);
    char charArray[]={'c', 'h', 'a', 'r'};
    std::vector<char> cc(charArray,charArray+4);
    short shortArray[] =  {3, 4, 5};
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
        portableArray[i] = TestNamedPortable(x,i);
    }
    std::vector<TestNamedPortable> nn(portableArray,portableArray + 5);
    
    TestInnerPortable inner(bb,cc,ss,ii,ll,ff,dd, nn);

    data = serializationService.toData(inner);
    
    TestInnerPortable tip1,tip2;
    tip1 = serializationService.toObject<TestInnerPortable>(data);
    tip2 = serializationService2.toObject<TestInnerPortable>(data);
    
    
    assert(inner == tip1);
    assert(inner == tip2);
    

    TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);
    data = serializationService.toData(main);
    
    TestMainPortable tmp1,tmp2;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
    tmp2 = serializationService2.toObject<TestMainPortable>(data);
    assert(main == tmp1);
    assert(main == tmp2);
 
    DataOutput* out = serializationService.pop();
    data.writeData(*out);
    std::vector<byte> outBuffer =  out->toByteArray();
    std::cout << outBuffer.size() << std::endl;
    std::cout << data.totalSize() << std::endl;
    ofstream outfile ("/Users/msk/Desktop/text.txt");
    for(int i = 0; i < outBuffer.size() ; i++)
        outfile.put(outBuffer[i]);
    
    serializationService.push(out);
    outfile.close();
   
};

TestMainPortable getTestMainPortable(){
    byte byteArray[]= {0, 1, 2};
    std::vector<byte> bb(byteArray,byteArray+3);
    char charArray[]={'c', 'h', 'a', 'r'};
    std::vector<char> cc(charArray,charArray+4);
    short shortArray[] =  {3, 4, 5};
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
        portableArray[i] = TestNamedPortable(x,i);
    }
    std::vector<TestNamedPortable> nn(portableArray,portableArray + 5);
    
    TestInnerPortable inner(bb,cc,ss,ii,ll,ff,dd, nn);
  TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);
    return main;
};