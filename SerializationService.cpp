//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationService.h"
#include "TypeSerializer.h"
#include "SerializationContext.h"
#include "ConstantSerializers.h"
#include "Portable.h"
#include "SerializationConstants.h"
using namespace std;

SerializationService::SerializationService(int version, PortableFactory* portableFactory)
                                                        : serializationContext(portableFactory,version,this)
                                                        , portableSerializer(&serializationContext)
{
    
};

SerializationService::~SerializationService(){
    while (!outputPool.empty())
    {
        DataOutput* out = outputPool.front();
        outputPool.pop();
        delete out;
    }
};

void SerializationService::push(DataOutput* out) {
    out->reset();
    outputPool.push(out);
};

DataOutput* SerializationService::pop() {
    DataOutput* out ;
    if (outputPool.empty()) {
        out = new DataOutput(this);
    }else{
        out = outputPool.front();
        outputPool.pop();
    }
    return out;
    
};

Data SerializationService::toData(bool object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    booleanSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_BOOLEAN, output->toByteArray());
    push(output);    
    return data;
};

Data SerializationService::toData(char object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_CHAR, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(short object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_SHORT, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(int object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_INTEGER, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(long object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_LONG, output->toByteArray());
    push(output);    
    return data;
    
};


Data SerializationService::toData(float object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_FLOAT, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(double object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE, output->toByteArray());
    push(output);   
    return data;
    
};

Data SerializationService::toData(Array<char>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(Array<short>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(Array<int>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};
Data SerializationService::toData(Array<long>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(Array<float>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(Array<double>& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(string& object){
    DataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    stringSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_STRING, output->toByteArray());
    push(output);    
    return data;
    
};

Data SerializationService::toData(Data& data){
    return data;
};

long SerializationService::combineToLong(int x, int y) {
    return ((long) x << 32) | ((long) y & 0xFFFFFFFL);
};

int SerializationService::extractInt(long value, bool lowerBits) {
    return (lowerBits) ? (int) value : (int) (value >> 32);
};

