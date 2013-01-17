//
// server.cpp

#include <iostream>
#include <vector>
#include <stdio.h>
#include <cassert>
#include "ContextAwareDataOutput.h"
#include "SerializationService.h"
#include "SerializationServiceImpl.h"
#include "TestPortableFactory.h"
#include "Data.h"
#include "ByteArray.h"

using namespace std;

int main(int argc, char* argv[])
{
    SerializationServiceImpl* serializationService = new SerializationServiceImpl(1,new TestPortableFactory());
    Data* data;
    
    int x = 3,y;
    data = serializationService->toData(x);
    y =  serializationService->toObject<int>(data);
    assert(x == y);
    
    CharArray bx(20),by;
    bx[2] = 5 + 'a';
    data = serializationService->toData(bx);
    by = serializationService->toObject<CharArray&>(data);
    cout << by[2] << endl;
    assert(bx == by);
    
    
    TestMainPortable portableX(34,4.32), portableY;
    data = serializationService->toData(portableX);
    cout << "Data size " << data->size() << endl;
    portableY =  serializationService->toObject<TestMainPortable>(data);
    
    assert(portableX == portableY );
    
    cout << "All tests are passed" << endl;
    return 0;
}