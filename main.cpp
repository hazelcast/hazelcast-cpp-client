//
// server.cpp

#include "TestPortableFactory.h"
#include "TestNamedPortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"

#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/Array.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "Server.h"
#include "Client.h"

#include <iostream>
#include <fstream>
#include <cassert>

using namespace hazelcast::client;

void write();
void read();
void client();

int main(int argc, char** argv){
    write();
    read();
//    client();
    return 0;
};

void client(){
    ClientConfig clientConfig;
    clientConfig.getGroupConfig().setName("myCluster").setPassword("sancar");
    clientConfig.setAddress("192.168.2.2:5701");
    
    try{
        auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);
//        auto_ptr< IMap<int,int> > map = hazelcastClient.getMap<int,int>("sancar");
//        map->put(1,2);
        
        std::cout << "Press a key to end" << std::endl;
        std::string x;
        std::cin >> x;
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
};

void read(){
    
    TestPortableFactory tpf1,tpf2;
    serialization::SerializationService serializationService(1, &tpf1);
    
    byte byteArray[]= {0, 1, 2};
    Array<byte> bb(3,byteArray);
    char charArray[]={'c', 'h', 'a', 'r'};
    Array<char> cc(4,charArray);
    short shortArray[] =  {3, 4, 5};
    Array<short> ss(3, shortArray);
    int integerArray[] = {9, 8, 7, 6};
    Array<int> ii(4, integerArray);
    long longArray[] = {0, 1, 5, 7, 9, 11};
    Array<long> ll(6, longArray);
    float floatArray[] = {0.6543f, -3.56f, 45.67f};
    Array<float> ff(3, floatArray);
    double doubleArray[] = {456.456, 789.789, 321.321};
    Array<double> dd(3, doubleArray);
    TestNamedPortable** portablePointerArray = new TestNamedPortable*[5];
    for (int i = 0; i < 5; i++) {
        string x = "named-portable-";
        x.push_back('0' + i);
        portablePointerArray[i] = new TestNamedPortable(x);
    }
    Array<Portable*> nn(5,(Portable**)portablePointerArray);
    
    TestInnerPortable inner(bb,cc,ss,ii,ll,ff,dd, nn);

    

    TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", &inner);
    
    
    
    std::ifstream is;
    is.open ("/Users/msk/Desktop/text.txt", std::ios::binary );
    char bytes[607];
    is.read(bytes,607);
    is.close();
    
    Array<byte> buffer(607,(byte*)bytes);
    serialization::DataInput dataInput(buffer,&serializationService);
    
    serialization::Data data;
    data.readData(dataInput);
    
    TestMainPortable tmp1;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
    assert(main == tmp1);
    
//    cout << "Read test is passed" << endl;
//    boost::asio::io_service client_service;
//    hazelcast::client client(client_service, "192.168.2.6", "8083",&serializationService);
//    client_service.run();
//    Data data;
//    client.read(data);
//    cout << data.size() << endl;
     
//    TestMainPortable tmp1;
//    tmp1 = serializationService.toObject<TestMainPortable>(data);

    for(int i = 0 ; i < 5 ; i++)
        delete portablePointerArray[i];
    delete [] portablePointerArray;
}

void write(){
    TestPortableFactory tpf1,tpf2;
    serialization::SerializationService serializationService(1, &tpf1);
//    serialization::SerializationService serializationService2(2, &tpf2);
    serialization::Data data;
    
    int x = 3;
    data = serializationService.toData(x);
    assert( x == serializationService.toObject<int>(data) );
    
    short f = 3.2;
    data = serializationService.toData(f);
    assert( f == serializationService.toObject<short>(data) );

    TestNamedPortable np("name");
    data = serializationService.toData(np);
    
    TestNamedPortable tnp1,tnp2;
    tnp1 = serializationService.toObject<TestNamedPortable>(data);
//    tnp2 = serializationService2.toObject<TestNamedPortable>(data);
    
    assert(np == tnp1);
//    assert(np == tnp2);
    
    byte byteArray[]= {0, 1, 2};
    Array<byte> bb(3,byteArray);
    char charArray[]={'c', 'h', 'a', 'r'};
    Array<char> cc(4,charArray);
    short shortArray[] =  {3, 4, 5};
    Array<short> ss(3, shortArray);
    int integerArray[] = {9, 8, 7, 6};
    Array<int> ii(4, integerArray);
    long longArray[] = {0, 1, 5, 7, 9, 11};
    Array<long> ll(6, longArray);
    float floatArray[] = {0.6543f, -3.56f, 45.67f};
    Array<float> ff(3, floatArray);
    double doubleArray[] = {456.456, 789.789, 321.321};
    Array<double> dd(3, doubleArray);
    TestNamedPortable** portablePointerArray = new TestNamedPortable*[5];
    for (int i = 0; i < 5; i++) {
        string x = "named-portable-";
        x.push_back('0' + i);
        portablePointerArray[i] = new TestNamedPortable(x);
    }
    Array<Portable*> nn(5,(Portable**)portablePointerArray);
    
    TestInnerPortable inner(bb,cc,ss,ii,ll,ff,dd, nn);

    
    data = serializationService.toData(inner);
    
    TestInnerPortable tip1,tip2;
    tip1 = serializationService.toObject<TestInnerPortable>(data);
//    tip2 = serializationService2.toObject<TestInnerPortable>(data);
    
    
    assert(inner == tip1);
//    assert(inner == tip2);
    

    TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", &inner);
    data = serializationService.toData(main);
    
    TestMainPortable tmp1,tmp2;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
//    tmp2 = serializationService2.toObject<TestMainPortable>(data);
    assert(main == tmp1);
//    assert(main == tmp2);
    
//    std::cout << "All tests are passed" << std::endl;
/*    
    DataOutput* out = serializationService.pop();
    data.writeData(*out);
    Array<byte> outBuffer =  out->toByteArray();

    ofstream outfile ("/Users/msk/Desktop/text.txt");
    for(int i = 0; i < outBuffer.length() ; i++)
        outfile.put(outBuffer[i]);
    
    serializationService.push(out);
    outfile.close();
//    boost::asio::io_service server_service;
//    hazelcast::server server(server_service, 8083,&serializationService);
//    server_service.run();
    
//    server.send(data);
*/    
   
    for(int i = 0 ; i < 5 ; i++)
        delete portablePointerArray[i];
    delete [] portablePointerArray;
}