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
#include "ConstantSerializers.h"
#include "PortableSerializer.h"
#include "ContextAwareDataOutput.h"
#include "ContextAwareDataInput.h"
#include "PortableFactory.h"

class SerializationServiceImpl {//: public SerializationService{ TODO
public:
    
    SerializationServiceImpl(int,PortableFactory*);
    ~SerializationServiceImpl();
    
    template<typename K>
    Data toData(K& object){
        ContextAwareDataOutput* output = pop();

        portableSerializer.write(output, object);
        
        Data data(SerializationConstants::CONSTANT_TYPE_PORTABLE, output->toByteArray());
        
        Portable* portable = dynamic_cast<Portable*>(&object);
        if (portable != NULL) {
            data.cd = serializationContext.lookup(portable->getClassId());
        }else{
            throw "class is not portable";
        }
        push(output);
        return data;

    };
    
    Data toData(Data&);
    Data toData(bool);
    Data toData(char);
    Data toData(short);
    Data toData(int);
    Data toData(long);
    Data toData(float);
    Data toData(double);
    Data toData(Array<char>&);
    Data toData(Array<short>&);
    Data toData(Array<int>&);
    Data toData(Array<long>&);
    Data toData(Array<float>&);
    Data toData(Array<double>&);
    Data toData(string&);
     
    template<typename K>
    inline K toObject(Data& data){
        if(data.size() == 0)
            throw "Empty Data";
        int typeID = data.type;
        if(typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE){
            serializationContext.registerClassDefinition(data.cd);
        }else{
            std::string error = "There is no suitable de-serializer for type ";
            error += typeID;
            throw error;
        }
        
        ContextAwareDataInput dataInput(data,this);
        K* obj = dynamic_cast<K*>(portableSerializer.read(dataInput).get());
        
        return *obj;
    };
    
    void push(ContextAwareDataOutput*);
    
    static long combineToLong(int x, int y);
    static int extractInt(long value, bool lowerBits);
    
    ContextAwareDataOutput* pop();
private:

    static int const OUTPUT_STREAM_BUFFER_SIZE = 32 * 1024;
    static int const CONSTANT_SERIALIZERS_SIZE = SerializationConstants::CONSTANT_SERIALIZERS_LENGTH;

    queue<ContextAwareDataOutput*> outputPool;
    
    PortableSerializer portableSerializer;
    ConstantSerializers::ByteSerializer byteSerializer;
    ConstantSerializers::BooleanSerializer booleanSerializer;
    ConstantSerializers::CharSerializer charSerializer;
    ConstantSerializers::ShortSerializer shortSerializer;
    ConstantSerializers::IntegerSerializer integerSerializer;
    ConstantSerializers::LongSerializer longSerializer;
    ConstantSerializers::FloatSerializer floatSerializer;
    ConstantSerializers::DoubleSerializer doubleSerializer;
    ConstantSerializers::ByteArraySerializer byteArraySerializer;
    ConstantSerializers::CharArraySerializer charArraySerializer;
    ConstantSerializers::ShortArraySerializer shortArraySerializer;
    ConstantSerializers::IntegerArraySerializer integerArraySerializer;
    ConstantSerializers::LongArraySerializer longArraySerializer;
    ConstantSerializers::FloatArraySerializer floatArraySerializer;
    ConstantSerializers::DoubleArraySerializer doubleArraySerializer;
    ConstantSerializers::StringSerializer stringSerializer;
    
    SerializationContextImpl serializationContext;
    

};

template<>
inline byte SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return byteSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline bool SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return booleanSerializer.read(dynamic_cast<DataInput*>(dataInput));
};


template<>
inline char SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return charSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline short SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return shortSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline int SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return integerSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline long SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return longSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline float SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return floatSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline double SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return doubleSerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<byte> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return byteArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<char> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return charArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<short> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return shortArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<int> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return integerArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<long> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return longArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<float> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return floatArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline Array<double> SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return doubleArraySerializer.read(dynamic_cast<DataInput*>(dataInput));
};

template<>
inline std::string SerializationServiceImpl::toObject(Data& data){
    if(data.size() == 0)
        throw "Empty Data";
    ContextAwareDataInput* dataInput = new ContextAwareDataInput(data,this);
    return stringSerializer.read(dynamic_cast<DataInput*>(dataInput));
};
#endif /* defined(__Server__SerializationServiceImpl__) */
