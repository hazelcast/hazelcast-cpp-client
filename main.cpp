//
// server.cpp

#include <iostream>
#include <vector>
#include <stdio.h>
#include <cassert>
#include "ContextAwareDataOutput.h"
#include "SerializationService.h"
#include "SerializationServiceImpl.h"
#include "Data.h"
#include "Array.h"

#include "TestPortableFactory.h"
#include "TestNamedPortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"
using namespace std;

int main(int argc, char** argv){
    TestPortableFactory tpf1,tpf2;
    SerializationServiceImpl serializationService(1, &tpf1);
    SerializationServiceImpl serializationService2(2, &tpf2);
    Data data;
    
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
    tnp2 = serializationService2.toObject<TestNamedPortable>(data);
    
    assert(np == tnp1);
    assert(np == tnp2);


//    TestNamedPortable* nn = new TestNamedPortable[5];
//    for (int i = 0; i < 5; i++) {
//        nn[i] = TestNamedPortable("named-portable-" + i);
//    }
    
//    Array<Portable> portablePointerArray(5,(Portable*)nn);
   
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
    
    TestInnerPortable inner(bb,cc,ss,ii,ll,ff,dd );

    
    data = serializationService.toData(inner);
    
    TestInnerPortable tip1,tip2;
    tip1 = serializationService.toObject<TestInnerPortable>(data);
    tip2 = serializationService2.toObject<TestInnerPortable>(data);
    
    
    assert(inner == tip1);
    assert(inner == tip2);
    

    TestMainPortable main((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", &inner);
    data = serializationService.toData(main);
    
    TestMainPortable tmp1,tmp2;
    tmp1 = serializationService.toObject<TestMainPortable>(data);
    tmp2 = serializationService2.toObject<TestMainPortable>(data);
    assert(main == tmp1);
    assert(main == tmp2);
     
    
    cout << "All tests are passed" << endl;
    return 0;
}