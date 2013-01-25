/* 
 * File:   main.cpp
 * Author: msk
 *
 * Created on January 24, 2013, 1:15 PM
 */

#include <cstdlib>
#include <fstream>
#include <cassert>
#include "SerializationServiceImpl.h"
#include "Data.h"
#include "Array.h"

#include "TestPortableFactory.h"
#include "TestMainPortable.h"

#include "Client.h"
using namespace std;

int main(int argc, char** argv) {

    TestPortableFactory tpf1,tpf2;
    SerializationServiceImpl serializationService(1, &tpf1);
    
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
    
    
    
    ifstream is;
    is.open ("/Users/msk/Desktop/text.txt", ios::binary );
    char bytes[607];
    is.read(bytes,607);
    is.close();
    
    Array<byte> buffer(607,(byte*)bytes);
    ContextAwareDataInput dataInput(buffer,&serializationService);
    
    Data data;
    data.readData(dataInput);
    
    TestMainPortable tmp1;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
    assert(main == tmp1);
    
    cout << "Read test is passed" << endl;
//    boost::asio::io_service client_service;
//    hazelcast::client client(client_service, "192.168.2.6", "8083",&serializationService);
//    client_service.run();
//    Data data;
//    client.read(data);
//    cout << data.size() << endl;
     
//    TestMainPortable tmp1;
//    tmp1 = serializationService.toObject<TestMainPortable>(data);
    return 0;
}

