//
// server.cpp


//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "ContextAwareDataOutput.h"
//#include "TypeSerializer.h"
#include "SerializationService.h"
#include "SerializationServiceImpl.h"
#include "TestPortableFactory.h"
#include "Data.h"
#include "ByteArray.h"

using namespace std;

int main(int argc, char* argv[])
{
    SerializationServiceImpl* serializationService = static_cast<SerializationServiceImpl*>(new SerializationServiceImpl(1,new TestPortableFactory()));
    Data* data;
    
    int x = 3,y;
    data = serializationService->toData(x);
    y =  serializationService->toObject<int>(data);
    if(x == y){
        cout << "OK" << endl;
    }else{
        cout << "FAIL" << endl;
    }
    
    TestMainPortable portableX, portableY;
    portableX.i = 10;
    data = serializationService->toData(portableX);
    portableY =  serializationService-> toObject<TestMainPortable>(data);
    
    if(portableX.i == portableY.i){
        cout << "OK" << endl;
    }else{
        cout << "FAIL" << endl;
        cout << "x:" << portableX.i << "=!" << "y:"<< portableY.i << endl;
    }

    return 0;
}