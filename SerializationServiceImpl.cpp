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
    registerDefault(typeid(Portable).name(),portableSerializer);
    registerDefault(typeid(byte).name(), new ConstantSerializers::ByteSerializer());
    registerDefault(typeid(bool).name(), new ConstantSerializers::BooleanSerializer());
    registerDefault(typeid(char).name(), new ConstantSerializers::CharSerializer());
    registerDefault(typeid(short).name(), new ConstantSerializers::ShortSerializer());
    registerDefault(typeid(int).name(), new ConstantSerializers::IntegerSerializer());
    registerDefault(typeid(long).name(), new ConstantSerializers::LongSerializer());
    registerDefault(typeid(float).name(), new ConstantSerializers::FloatSerializer());
    registerDefault(typeid(double).name(), new ConstantSerializers::DoubleSerializer());
    registerDefault(typeid(byte*).name(), new ConstantSerializers::ByteArraySerializer());
    registerDefault(typeid(char*).name(), new ConstantSerializers::CharArraySerializer());
    registerDefault(typeid(short*).name(), new ConstantSerializers::ShortArraySerializer());
    registerDefault(typeid(int*).name(), new ConstantSerializers::IntegerArraySerializer());
    registerDefault(typeid(long*).name(), new ConstantSerializers::LongArraySerializer());
    registerDefault(typeid(float*).name(), new ConstantSerializers::FloatArraySerializer());
    registerDefault(typeid(double*).name(), new ConstantSerializers::DoubleArraySerializer());
    registerDefault(typeid(string).name(), new ConstantSerializers::StringSerializer());
    
};

SerializationServiceImpl::~SerializationServiceImpl(){
    /*
    for (ContextAwareDataOutput output : outputPool) {
        output.close();
    }
    outputPool.clear();
     */
};

void SerializationServiceImpl::push(ContextAwareDataOutput out) {
//    out.reset();
//    outputPool.offer(out);
};

//template <typename T>//TODO i added template
TypeSerializer* SerializationServiceImpl::serializerFor(int const typeId) {
//    if (typeId < 0) {
//        final int index = indexForDefaultType(typeId);
//        if (index < CONSTANT_SERIALIZERS_SIZE) {
//            return constantTypeIds[index];
//        }
//    }
//    return NULL;
};

template <typename T>
TypeSerializer* SerializationServiceImpl::serializerFor(T object) {
    
    if(dynamic_cast<Portable*>(object)){
        return portableSerializer;
    }
    
    TypeSerializer* serializer = constantsTypeMap[typeid(object).name()];
    if (serializer != NULL) {
       return serializer;
    };
    
    return NULL;
    
};

SerializationContext* SerializationServiceImpl::getSerializationContext() {
//    return serializationContext;
};

ContextAwareDataOutput SerializationServiceImpl::pop() {
    /*
    ContextAwareDataOutput out = outputPool.poll();
    if (out == NULL) {
        out = new ContextAwareDataOutput(OUTPUT_STREAM_BUFFER_SIZE, this);
    }
    return out;
     */
}

//template <typename T>//TODO i added template
void SerializationServiceImpl::registerDefault(std::string typeId, TypeSerializer* serializer) {
    constantsTypeMap[typeId] = serializer;
    constantTypeIds[indexForDefaultType(serializer->getTypeId())] = serializer;
};

int SerializationServiceImpl::indexForDefaultType(int const typeId) {
    return -typeId - 1;
};



