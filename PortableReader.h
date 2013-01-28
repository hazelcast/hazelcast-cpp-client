//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__PortableReader__
#define __Server__PortableReader__

#include <iostream>
#include <string>
#include <memory>
#include "Array.h"
#include "ClassDefinition.h"
#include "DataInput.h"
#include "FieldDefinition.h"
#include "PortableSerializer.h"
class Portable;
//class PortableSerializer;
class BufferObjectDataInput;
//class FieldDefinition;

typedef unsigned char byte;

using namespace std;

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
    
    template<typename T>
    auto_ptr<T> readPortable(string fieldName) throw(ios_base::failure) {
        if(!cd->isFieldDefinitionExists(fieldName))
           throw "throwUnknownFieldException" + fieldName;

        FieldDefinition fd = cd->get(fieldName);

        int pos = getPosition(&fd);
        input->position(pos);
        bool isNull = input->readBoolean();
        if (isNull) {//TODO search for return NULL
            auto_ptr<T> x;
            return x;
        }
        input->setDataClassId(fd.getClassId());
        auto_ptr<T> p ((T*)serializer->read(*input).release() );

        input->setDataClassId(cd->getClassId());
        return p;
    };
    
    Array<byte> readByteArray(string) throw(ios_base::failure);
    
    Array<char> readCharArray(string) throw(ios_base::failure);
    
    Array<int> readIntArray(string) throw(ios_base::failure);
    
    Array<long> readLongArray(string) throw(ios_base::failure);
    
    Array<double> readDoubleArray(string) throw(ios_base::failure);
    
    Array<float> readFloatArray(string) throw(ios_base::failure);
    
    Array<short> readShortArray(string) throw(ios_base::failure);
    
    template<typename T>
    Array< auto_ptr<T> > readPortableArray(string fieldName) throw(ios_base::failure){//TODO
        if(!cd->isFieldDefinitionExists(fieldName))
            throw "throwUnknownFieldException" + fieldName;
        FieldDefinition fd = cd->get(fieldName);

        int pos = getPosition(fieldName);
        input->position(pos);
        int len = input->readInt();
        Array< auto_ptr<T> > portables(len);
        input->setDataClassId(fd.getClassId());
        for (int i = 0; i < len; i++) {
            portables[i] = auto_ptr<T>((T*)serializer->read(*input).release());
        }
        return portables;
    };

protected:
    int getPosition(string) throw(ios_base::failure);

    int getPosition(FieldDefinition*) throw(ios_base::failure);
    
    PortableSerializer* serializer;
    ClassDefinition* cd;
    DataInput* input;
    int offset;
    
};


#endif /* defined(__Server__PortableReader__) */
