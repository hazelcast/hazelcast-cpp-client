//
// server.cpp

#include <iostream>
#include <vector>
#include <stdio.h>
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
    if(x == y){
        cout << "OK" << endl;
    }else{
        cout << "FAIL" << endl;
    }
    
    TestMainPortable portableX(34,4.32), portableY;
    
    data = serializationService->toData(portableX);
    cout << "Data size " << data->size() << endl;
    portableY =  serializationService->toObject<TestMainPortable>(data);
    
    if(portableX == portableY ){
        cout << "OK" << endl;
    }else{
        cout << "FAIL" << endl;
        cout << "x:i:" << portableX.i << "=!" << "y:i:"<< portableY.i << endl;
        cout << "x:f:" << portableX.f << "=!" << "y:f:"<< portableY.f << endl;
    }

    return 0;
}