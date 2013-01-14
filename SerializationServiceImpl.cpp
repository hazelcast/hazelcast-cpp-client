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


Data* SerializationServiceImpl::toData(int object){
    ContextAwareDataOutput* output = pop();
    if(output == NULL){
        
    }
    DataOutput* dataOutput = dynamic_cast<DataOutput*>(output);
    integerSerializer->write(dataOutput, object);
    
    Data* data = new Data();
    //        = new Data(serializer->getTypeId(), output->toByteArray());
    return data;
    
};

Data* SerializationServiceImpl::toData(Data& data){
    return &data;
};


