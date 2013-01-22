//
//  SerializationServiceImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationServiceImpl.h"
#include "TypeSerializer.h"
#include "SerializationContextImpl.h"
#include "ConstantSerializers.h"
#include "Portable.h"
#include "SerializationConstants.h"
using namespace std;

SerializationServiceImpl::SerializationServiceImpl(int version, PortableFactory* portableFactory)
                                                        : serializationContext(portableFactory,version,this)
                                                        , portableSerializer(&serializationContext)
{
    
};

SerializationServiceImpl::~SerializationServiceImpl(){
    while (!outputPool.empty())
    {
        outputPool.pop();
    }
};

void SerializationServiceImpl::push(ContextAwareDataOutput* out) {
    out->reset();
    outputPool.push(out);
};

ContextAwareDataOutput* SerializationServiceImpl::pop() {
    ContextAwareDataOutput* out ;
    if (outputPool.empty()) {
        out = new ContextAwareDataOutput(this);
    }else{
        out = outputPool.front();
        out->reset();
        outputPool.pop();
    }
    return out;
    
};

Data SerializationServiceImpl::toData(bool object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    booleanSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_BOOLEAN, output->toByteArray());
    return data;
};

Data SerializationServiceImpl::toData(char object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_CHAR, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(short object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_SHORT, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(int object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_INTEGER, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(long object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_LONG, output->toByteArray());
    return data;
    
};


Data SerializationServiceImpl::toData(float object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_FLOAT, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(double object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Array<char>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Array<short>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Array<int>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY, output->toByteArray());
    return data;
    
};
Data SerializationServiceImpl::toData(Array<long>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Array<float>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Array<double>& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleArraySerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(string& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    stringSerializer.write(dataOutput, object);
    
    Data data(SerializationConstants::CONSTANT_TYPE_STRING, output->toByteArray());
    return data;
    
};

Data SerializationServiceImpl::toData(Data& data){
    return data;
};

long SerializationServiceImpl::combineToLong(int x, int y) {
    return ((long) x << 32) | ((long) y & 0xFFFFFFFL);
}

int SerializationServiceImpl::extractInt(long value, bool lowerBits) {
    return (lowerBits) ? (int) value : (int) (value >> 32);
}

