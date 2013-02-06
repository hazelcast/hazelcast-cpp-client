//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_SERVICE
#define HAZELCAST_SERIALIZATION_SERVICE

#include "Data.h"
#include "ConstantSerializers.h"
#include "PortableSerializer.h"
#include "DataOutput.h"
#include "DataInput.h"
#include "PortableFactory.h"
#include "SerializationService.h"
#include "SerializationContext.h"
#include <iostream>
#include <string>
#include <map>
#include <queue>

namespace hazelcast{ 
namespace client{
namespace serialization{

class SerializationService{ 
public:
    
    SerializationService(int,PortableFactory const *);
    ~SerializationService();
    
    template<typename K>
    Data toData(K& object){
        DataOutput* output = pop();

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
    Data toData(std::vector<char>&);
    Data toData(std::vector<short>&);
    Data toData(std::vector<int>&);
    Data toData(std::vector<long>&);
    Data toData(std::vector<float>&);
    Data toData(std::vector<double>&);
    Data toData(string&);
     
    template<typename K>
    inline K toObject(const Data& data){
        if(data.bufferSize() == 0)
            throw "Empty Data";
        int typeID = data.type;
        if(typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE){
            serializationContext.registerClassDefinition(data.cd);
        }else{
            std::string error = "There is no suitable de-serializer for type ";
            error += typeID;
            throw error;
        }
        
        DataInput dataInput(data,this);
        std::auto_ptr<Portable> autoPtr(portableSerializer.read(dataInput));
        
        K* ptr = dynamic_cast<K*>( autoPtr.get() );
       
        return (*ptr);
    };
    
    void push(DataOutput*);
    
    DataOutput* pop();
    
    static long combineToLong(int x, int y);
    static int extractInt(long value, bool lowerBits);
    
    SerializationContext* getSerializationContext(){
        return &serializationContext;
    }
private:

    static int const OUTPUT_STREAM_BUFFER_SIZE = 32 * 1024;
    static int const CONSTANT_SERIALIZERS_SIZE = SerializationConstants::CONSTANT_SERIALIZERS_LENGTH;

    queue<DataOutput*> outputPool;
    
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
    
    SerializationContext serializationContext;
    

};

template<>
inline byte SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return byteSerializer.read(&dataInput);
};

template<>
inline bool SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return booleanSerializer.read(&dataInput);
};


template<>
inline char SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return charSerializer.read(&dataInput);
};

template<>
inline short SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return shortSerializer.read(&dataInput);
};

template<>
inline int SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return integerSerializer.read(&dataInput);
};

template<>
inline long SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return longSerializer.read(&dataInput);
};

template<>
inline float SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return floatSerializer.read(&dataInput);
};

template<>
inline double SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return doubleSerializer.read(&dataInput);
};

template<>
inline std::vector<byte> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return byteArraySerializer.read(&dataInput);
};

template<>
inline std::vector<char> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return charArraySerializer.read(&dataInput);
};

template<>
inline std::vector<short> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return shortArraySerializer.read(&dataInput);
};

template<>
inline std::vector<int> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return integerArraySerializer.read(&dataInput);
};

template<>
inline std::vector<long> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return longArraySerializer.read(&dataInput);
};

template<>
inline std::vector<float> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return floatArraySerializer.read(&dataInput);
};

template<>
inline std::vector<double> SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return doubleArraySerializer.read(&dataInput);
};

template<>
inline std::string SerializationService::toObject(const Data& data){
    if(data.bufferSize() == 0)
        throw "Empty Data";
    DataInput dataInput(data,this);
    return stringSerializer.read(&dataInput);
};

}}}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
