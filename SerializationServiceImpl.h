//
//  SerializationServiceImpl.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__SerializationServiceImpl__
#define __Server__SerializationServiceImpl__

#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <typeinfo>
#include "Data.h"
#include "PortableFactory.h"
#include "ShortArray.h"
#include "CharArray.h"
#include "IntegerArray.h"
#include "LongArray.h"
#include "FloatArray.h"
#include "DoubleArray.h"
#include "ConstantSerializers.h"
#include "PortableSerializer.h"
#include "ContextAwareDataOutput.h"
#include "ContextAwareDataInput.h"

class SerializationServiceImpl{// : public SerializationService{
public:
    
    SerializationServiceImpl(int,PortableFactory*);
    ~SerializationServiceImpl();
    
    template<typename K>
    Data* toData(K object){
        ContextAwareDataOutput* output = pop();

        portableSerializer->write(output, &object);
        
        Data* data = new Data(SerializationConstants::CONSTANT_TYPE_PORTABLE, output->toByteArray());
        
        Portable* portable = dynamic_cast<Portable*>(&object);
        if (portable != NULL) {
            data->cd = serializationContext->lookup(portable->getClassId());
        }else{
            push(output);
            throw "class is not portable";
        }
        return data;

    };
    
    Data* toData(Data&);
    Data* toData(bool);
    Data* toData(char);
    Data* toData(short);
    Data* toData(int);
    Data* toData(long);
    Data* toData(float);
    Data* toData(double);
    Data* toData(CharArray&);
    Data* toData(ShortArray&);
    Data* toData(IntegerArray&);
    Data* toData(LongArray&);
    Data* toData(FloatArray&);
    Data* toData(DoubleArray&);
    Data* toData(string&);
    
    template<typename K>
    inline K toObject(Data* data){
        if(data == NULL || data->size() == 0)
            throw "Null pointer exception";
        int typeID = data->type;
        if(typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE){
            serializationContext->registerClassDefinition(data->cd);
        }else{
            std::string error = "There is no suitable de-serializer for type ";
            error += typeID;
            throw error;
        }
        
        ContextAwareDataInput* dataInput = new ContextAwareDataInput(*data,this);
        K* obj = (K*)portableSerializer->read(dataInput);
        
        return *obj;
    };
    
    void push(ContextAwareDataOutput*);
    
//    TypeSerializer* serializerFor(int const);
    
    SerializationContextImpl* getSerializationContext();
    static long combineToLong(int x, int y);
    static int extractInt(long value, bool lowerBits);
    
    ContextAwareDataOutput* pop();
private:

//    void registerDefault(TypeSerializer*);
    
    int indexForDefaultType(int const);
    
    
    static int const OUTPUT_STREAM_BUFFER_SIZE = 32 * 1024;
    static int const CONSTANT_SERIALIZERS_SIZE = SerializationConstants::CONSTANT_SERIALIZERS_LENGTH;

    TypeSerializer** constantTypeIds ;
    
    queue<ContextAwareDataOutput*> outputPool;
    /*
    TypeSerializer* portableSerializer;
    TypeSerializer* byteSerializer;
    TypeSerializer* booleanSerializer;
    TypeSerializer* charSerializer;
    TypeSerializer* shortSerializer;
    TypeSerializer* integerSerializer;
    TypeSerializer* longSerializer;
    TypeSerializer* floatSerializer;
    TypeSerializer* doubleSerializer;
    TypeSerializer* byteArraySerializer;
    TypeSerializer* charArraySerializer;
    TypeSerializer* shortArraySerializer;
    TypeSerializer* integerArraySerializer;
    TypeSerializer* longArraySerializer;
    TypeSerializer* floatArraySerializer;
    TypeSerializer* doubleArraySerializer;
    TypeSerializer* stringSerializer;
     */
    
    PortableSerializer* portableSerializer;
    ConstantSerializers::ByteSerializer* byteSerializer;
    ConstantSerializers::BooleanSerializer* booleanSerializer;
    ConstantSerializers::CharSerializer* charSerializer;
    ConstantSerializers::ShortSerializer* shortSerializer;
    ConstantSerializers::IntegerSerializer* integerSerializer;
    ConstantSerializers::LongSerializer* longSerializer;
    ConstantSerializers::FloatSerializer* floatSerializer;
    ConstantSerializers::DoubleSerializer* doubleSerializer;
    ConstantSerializers::ByteArraySerializer* byteArraySerializer;
    ConstantSerializers::CharArraySerializer* charArraySerializer;
    ConstantSerializers::ShortArraySerializer* shortArraySerializer;
    ConstantSerializers::IntegerArraySerializer* integerArraySerializer;
    ConstantSerializers::LongArraySerializer* longArraySerializer;
    ConstantSerializers::FloatArraySerializer* floatArraySerializer;
    ConstantSerializers::DoubleArraySerializer* doubleArraySerializer;
    ConstantSerializers::StringSerializer* stringSerializer;
    
    SerializationContextImpl* serializationContext;
    

};

template<>
inline int SerializationServiceImpl::toObject(Data* data){
    if(data == NULL || data->size() == 0)
        throw "Null pointer exception";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(*data,this);
    return integerSerializer->read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline float SerializationServiceImpl::toObject(Data* data){
    if(data == NULL || data->size() == 0)
        throw "Null pointer exception";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(*data,this);
    return floatSerializer->read(dynamic_cast<DataInput*>(dataInput));
};


#endif /* defined(__Server__SerializationServiceImpl__) */
