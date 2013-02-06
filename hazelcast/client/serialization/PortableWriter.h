 //
//  PortableWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_WRITER
#define HAZELCAST_PORTABLE_WRITER

#include "DataOutput.h"
#include "PortableSerializer.h"
#include <iostream>
#include <string>
using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{
    
class ClassDefinition;
class DataInput;
class Portable;

class PortableWriter{
public:
    PortableWriter();
    
    PortableWriter(PortableSerializer* serializer, DataOutput* output, ClassDefinition* cd);
        
    void writeInt(string fieldName, int value);
    
    void writeLong(string fieldName, long value);
    
    void writeBoolean(string fieldName, bool value);
    
    void writeByte(string fieldName, byte value);
    
    void writeChar(string fieldName, int value);
    
    void writeDouble(string fieldName, double value);
    
    void writeFloat(string fieldName, float value);
    
    void writeShort(string fieldName, short value);
    
    void writeUTF(string fieldName, string str);
    
    void writePortable(string fieldName, Portable& portable);
    
    void writeByteArray(string fieldName, std::vector<byte>&);
    
    void writeCharArray(string fieldName, std::vector<char>&);
    
    void writeIntArray(string fieldName, std::vector<int>&);
    
    void writeLongArray(string fieldName, std::vector<long>&);
    
    void writeDoubleArray(string fieldName, std::vector<double>&);
    
    void writeFloatArray(string fieldName, std::vector<float>&);
    
    void writeShortArray(string fieldName, std::vector<short>&);
    
    template<typename T>
    void writePortableArray(string fieldName, std::vector<T>& portables) {
        setPosition(fieldName);
        int len = portables.size();
        output->writeInt(len);
        if (len > 0) {
            int offset = output->position();
            char zeros[len * sizeof (int) ];
            output->write(zeros, 0, len * sizeof (int));
            for (int i = 0; i < len; i++) {
                output->writeInt(offset + i * sizeof (int), output->position());
                serializer->write(output, portables[i]);
            }
        }
    };
    void setPosition(string fieldName);
    
private:
    PortableSerializer* serializer;

    ClassDefinition* cd;
    DataOutput* output;
    int offset;
};

}}}
#endif /* HAZELCAST_PORTABLE_WRITER */
