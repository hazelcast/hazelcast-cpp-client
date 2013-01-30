//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_READER
#define HAZELCAST_PORTABLE_READER

#include "ClassDefinition.h"
#include "DataInput.h"
#include "FieldDefinition.h"
#include "PortableSerializer.h"
#include "../Array.h"
#include <iostream>
#include <string>
#include <memory>

using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{

class Portable;
class BufferObjectDataInput;

typedef unsigned char byte;

class PortableReader{
public:
    PortableReader(PortableSerializer*, DataInput&, ClassDefinition*);
    
    int readInt(string);
    
    long readLong(string);
    
    bool readBoolean(string);
    
    byte readByte(string);
    
    char readChar(string);
    
    double readDouble(string);
    
    float readFloat(string);
    
    short readShort(string);
    
    string readUTF(string);
    
    auto_ptr<Portable> readPortable(string fieldName);
    
    Array<byte> readByteArray(string);
    
    Array<char> readCharArray(string);
    
    Array<int> readIntArray(string);
    
    Array<long> readLongArray(string);
    
    Array<double> readDoubleArray(string);
    
    Array<float> readFloatArray(string);
    
    Array<short> readShortArray(string);
    
    Array< auto_ptr<Portable> > readPortableArray(string fieldName);
    

protected:
    int getPosition(string);

    int getPosition(FieldDefinition*);
    
    PortableSerializer* serializer;
    ClassDefinition* cd;
    DataInput* input;
    int offset;
    
};

}}}
#endif /* HAZELCAST_PORTABLE_READER */
