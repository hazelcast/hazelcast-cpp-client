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
    
    int readInt(string) throw(ios_base::failure);
    
    long readLong(string) throw(ios_base::failure);
    
    bool readBoolean(string) throw(ios_base::failure);
    
    byte readByte(string) throw(ios_base::failure);
    
    char readChar(string) throw(ios_base::failure);
    
    double readDouble(string) throw(ios_base::failure);
    
    float readFloat(string) throw(ios_base::failure);
    
    short readShort(string) throw(ios_base::failure);
    
    string readUTF(string) throw(ios_base::failure);
    
    auto_ptr<Portable> readPortable(string fieldName) throw(ios_base::failure);
    
    Array<byte> readByteArray(string) throw(ios_base::failure);
    
    Array<char> readCharArray(string) throw(ios_base::failure);
    
    Array<int> readIntArray(string) throw(ios_base::failure);
    
    Array<long> readLongArray(string) throw(ios_base::failure);
    
    Array<double> readDoubleArray(string) throw(ios_base::failure);
    
    Array<float> readFloatArray(string) throw(ios_base::failure);
    
    Array<short> readShortArray(string) throw(ios_base::failure);
    
    Array< auto_ptr<Portable> > readPortableArray(string fieldName) throw(ios_base::failure);
    

protected:
    int getPosition(string) throw(ios_base::failure);

    int getPosition(FieldDefinition*) throw(ios_base::failure);
    
    PortableSerializer* serializer;
    ClassDefinition* cd;
    DataInput* input;
    int offset;
    
};

}}}
#endif /* HAZELCAST_PORTABLE_READER */
