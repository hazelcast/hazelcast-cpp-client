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
#include "ByteArray.h"

#include "TestPortableFactory.h"
#include "TestNamedPortable.h"
#include "TestInnerPortable.h"
#include "TestMainPortable.h"
using namespace std;

int main(int argc, char* argv){
    
    SerializationServiceImpl* serializationService = new SerializationServiceImpl(1, new TestPortableFactory());
    SerializationServiceImpl* serializationService2 = new SerializationServiceImpl(2, new TestPortableFactory());
    Data* data;
    
    int x = 3;
    data = serializationService->toData(x);
    assert( x == serializationService->toObject<int>(data) );
//    delete data;
    
    TestNamedPortable** nn = new TestNamedPortable*[5];
    for (int i = 0; i < 5; i++) {
        nn[i] = new TestNamedPortable("named-portable-" + i);
    }

    TestNamedPortable* np = nn[0];
    data = serializationService->toData(*np);
    
    TestNamedPortable tnp1,tnp2;
    tnp1 = serializationService->toObject<TestNamedPortable>(data);
    tnp2 = serializationService2->toObject<TestNamedPortable>(data);
    
    assert(*np == tnp1);
    assert(*np == tnp2);

    byte byteArray[]= {0, 1, 2};
    ByteArray bb(3,byteArray);
    
    PortablePointerArray portablePointerArray(5,(Portable**)nn);
    
    for (int i = 0; i < 5; i++) {
        delete nn[i];
    }
    
    char cc[]={'c', 'h', 'a', 'r'};
    short ss[] =  {3, 4, 5};
    int ii[] = {9, 8, 7, 6};
    long ll[] = {0, 1, 5, 7, 9, 11};
    float ff[] = {0.6543f, -3.56f, 45.67f};
    double dd[] = {456.456, 789.789, 321.321};
    TestInnerPortable* inner = new TestInnerPortable(&bb,&portablePointerArray , cc,ss,ii,ll,ff,dd, 4, 3, 4, 6, 3, 3);

    
    data = serializationService->toData(*inner);
    TestInnerPortable tip1,tip2;
    tip1 = serializationService->toObject<TestMainPortable>(data);
    tip2 = serializationService2->toObject<TestMainPortable>(data);
            
    assert(*inner == tip1);
    assert(*inner == tip2);

    TestMainPortable* main = new TestMainPortable((byte) 113, true, 'x', (short) -500, 56789, -50992225, 900.5678,
            -897543.3678909, "this is main portable object created for testing!", inner);

    delete inner;
    
    data = serializationService->toData(*main);
    TestMainPortable tmp1,tmp2;
    tmp1 = serializationService->toObject<TestMainPortable>(data);
    tmp2 = serializationService2->toObject<TestMainPortable>(data);
    assert(*main == tmp1);
    assert(*main == tmp2);
    
    delete main;
    delete serializationService;
    delete serializationService2;
    
    cout << "All tests are passed" << endl;
    return 0;
}