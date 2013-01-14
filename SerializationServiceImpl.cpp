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

SerializationServiceImpl::SerializationServiceImpl(int version, PortableFactory* portableFactory) {
    
    constantTypeIds = new TypeSerializer*[SerializationConstants::CONSTANT_SERIALIZERS_LENGTH];
    
    serializationContext = new SerializationContextImpl(portableFactory, version);
    
    portableSerializer = new PortableSerializer(serializationContext);
    byteSerializer = new ConstantSerializers::ByteSerializer();
    booleanSerializer = new ConstantSerializers::BooleanSerializer();
    charSerializer = new ConstantSerializers::CharSerializer();
    shortSerializer = new ConstantSerializers::ShortSerializer();
    integerSerializer = new ConstantSerializers::IntegerSerializer();
    longSerializer = new ConstantSerializers::LongSerializer();
    floatSerializer = new ConstantSerializers::FloatSerializer();
    doubleSerializer = new ConstantSerializers::DoubleSerializer();
    byteArraySerializer = new ConstantSerializers::ByteArraySerializer();
    charArraySerializer = new ConstantSerializers::CharArraySerializer();
    shortArraySerializer = new ConstantSerializers::ShortArraySerializer();
    integerArraySerializer = new ConstantSerializers::IntegerArraySerializer();
    longArraySerializer = new ConstantSerializers::LongArraySerializer();
    floatArraySerializer = new ConstantSerializers::FloatArraySerializer();
    doubleArraySerializer = new ConstantSerializers::DoubleArraySerializer();
    stringSerializer = new ConstantSerializers::StringSerializer();
    
    registerDefault(portableSerializer);
    registerDefault(byteSerializer);
    registerDefault(booleanSerializer);
    registerDefault(charSerializer);
    registerDefault(shortSerializer);
    registerDefault(integerSerializer);
    registerDefault(longSerializer);
    registerDefault(floatSerializer);
    registerDefault(doubleSerializer);
    registerDefault(byteArraySerializer);
    registerDefault(charArraySerializer);
    registerDefault(shortArraySerializer);
    registerDefault(integerArraySerializer);
    registerDefault(longArraySerializer);
    registerDefault(floatArraySerializer);
    registerDefault(doubleArraySerializer);
    registerDefault(stringSerializer);
    
};

SerializationServiceImpl::~SerializationServiceImpl(){
    /*
    for (ContextAwareDataOutput output : outputPool) {
        output.close();
    }
     */
    while (!outputPool.empty())
    {
        outputPool.pop();
    }
    
};

void SerializationServiceImpl::push(ContextAwareDataOutput* out) {
//    out->reset();
    outputPool.push(out);
};

TypeSerializer* SerializationServiceImpl::serializerFor(int const typeId) {
    if (typeId < 0) {
        int index = indexForDefaultType(typeId);
        if (index < CONSTANT_SERIALIZERS_SIZE) {
            return constantTypeIds[index];
        }
    }
    return NULL;
};

SerializationContext* SerializationServiceImpl::getSerializationContext() {
    return serializationContext;
};

ContextAwareDataOutput* SerializationServiceImpl::pop() {
    ContextAwareDataOutput* out = outputPool.front();
    outputPool.pop();
    if (out == NULL) {
        out = new ContextAwareDataOutput(this); //TODO change with the next line
//        out = new ContextAwareDataOutput(OUTPUT_STREAM_BUFFER_SIZE, this);
    }
    return out;

};

void SerializationServiceImpl::registerDefault(TypeSerializer* serializer) {
    constantTypeIds[indexForDefaultType(serializer->getTypeId())] = serializer;
};

int SerializationServiceImpl::indexForDefaultType(int const typeId) {
    return -typeId - 1;
};


Data* SerializationServiceImpl::toData(bool object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    booleanSerializer->write(dataOutput, object);
    
    Data* data = new Data(booleanSerializer->getTypeId(), output->toByteArray());
    return data;
};

Data* SerializationServiceImpl::toData(char object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charSerializer->write(dataOutput, object);
    
    Data* data = new Data(charSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(short object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortSerializer->write(dataOutput, object);
    
    Data* data = new Data(shortSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(int object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerSerializer->write(dataOutput, object);
    
    Data* data = new Data(integerSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(long object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longSerializer->write(dataOutput, object);
    
    Data* data = new Data(longSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(float object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatSerializer->write(dataOutput, object);
    
    Data* data = new Data(floatSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(double object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleSerializer->write(dataOutput, object);
    
    Data* data = new Data(doubleSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(CharArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    charArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(charArraySerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(ShortArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    shortArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(shortArraySerializer->getTypeId(), output->toByteArray());
    return data;

};

Data* SerializationServiceImpl::toData(IntegerArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(integerArraySerializer->getTypeId(), output->toByteArray());
    return data;
    
};
Data* SerializationServiceImpl::toData(LongArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    longArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(longArraySerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(FloatArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    floatArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(floatArraySerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(DoubleArray& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    doubleArraySerializer->write(dataOutput, object);
    
    Data* data = new Data(doubleArraySerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(string& object){
    ContextAwareDataOutput* output = pop();
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    stringSerializer->write(dataOutput, object);
    
    Data* data = new Data(stringSerializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(Data& data){
    return &data;
};


