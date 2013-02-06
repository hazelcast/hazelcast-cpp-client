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
    
    std::vector<byte> readByteArray(string);
    
    std::vector<char> readCharArray(string);
    
    std::vector<int> readIntArray(string);
    
    std::vector<long> readLongArray(string);
    
    std::vector<double> readDoubleArray(string);
    
    std::vector<float> readFloatArray(string);
    
    std::vector<short> readShortArray(string);
    
    template<typename T>
    T readPortable(string fieldName){
        if(!cd->isFieldDefinitionExists(fieldName))
           throw "throwUnknownFieldException" + fieldName;

        FieldDefinition fd = cd->get(fieldName);

        int pos = getPosition(&fd);
        input->position(pos);
        bool isNull = input->readBoolean();
        if (isNull) {
            return T();
        }
        input->setDataClassId(fd.getClassId());
        std::auto_ptr<Portable> p (serializer->read(*input) );

        input->setDataClassId(cd->getClassId());
        T portable = *dynamic_cast<T*>(p.get());
        return portable;
    };
    
    template<typename T>
    std::vector< T > readPortableArray(string fieldName){
        if(!cd->isFieldDefinitionExists(fieldName))
              throw "throwUnknownFieldException" + fieldName;
          FieldDefinition fd = cd->get(fieldName);
          int currentPos = input->position();
          int pos = getPosition(fieldName);
          input->position(pos);
          int len = input->readInt();
          std::vector< T > portables(len);
          if(len > 0 ){
            int offset = input->position();
            input->setDataClassId(fd.getClassId());
            int start;
            for (int i = 0; i < len; i++) {
                start = input->readInt(offset + i * sizeof(int));
                input->position(start);
                portables[i] = *dynamic_cast<T*>(serializer->read(*input).get());
            }
          }
          input->position(currentPos);
          return portables;
    };
    

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
