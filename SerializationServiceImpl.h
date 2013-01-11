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
#include "SerializationService.h"
#include "SerializationContext.h"
#include "PortableFactory.h"
#include "SerializationConstants.h"
#include "ContextAwareDataOutput.h"
#include "TypeSerializer.h"
#include "PortableSerializer.h"

class SerializationServiceImpl :public SerializationService{
public:
    
    SerializationServiceImpl(int,PortableFactory*);
    ~SerializationServiceImpl();
    
    template<typename K>
    Data toData(K){
        /*
         ContextAwareDataOutput out = pop();
         try {
         final TypeSerializer serializer = serializerFor(obj.getClass());
         if (serializer == NULL) {
         throw new NotSerializableException("There is no suitable serializer for " + obj.getClass());
         }
         serializer.write(out, obj);
         final Data data = new Data(serializer.getTypeId(), out.toByteArray());
         if (obj instanceof Portable) {
         data.cd = serializationContext.lookup(((Portable) obj).getClassId());
         }
         return data;
         } catch (Throwable e) {
         if (e instanceof HazelcastSerializationException) {
         throw (HazelcastSerializationException) e;
         }
         throw new HazelcastSerializationException(e);
         } finally {
         push(out);
         }
         */
        Data d;
        return d;
    };
    
    template<typename K>
    K toObject(Data){
        K b;
        return b;
        /*
         if ((data == NULL) || (data.buffer == NULL) || (data.buffer.length == 0)) {
         return NULL;
         }
         ContextAwareDataInput in = NULL;
         try {
         final int typeId = data.type;
         final TypeSerializer serializer = serializerFor(typeId);
         if (serializer == NULL) {
         throw new IllegalArgumentException("There is no suitable de-serializer for type " + typeId);
         }
         if (data.type == SerializationConstants.CONSTANT_TYPE_PORTABLE) {
         serializationContext.registerClassDefinition(data.cd);
         }
         in = new ContextAwareDataInput(data, this);
         Object obj = serializer.read(in);
         return obj;
         } catch (Throwable e) {
         if (e instanceof HazelcastSerializationException) {
         throw (HazelcastSerializationException) e;
         }
         throw new HazelcastSerializationException(e);
         } finally {
         if (in != NULL) {
         in.close();
         }
         }
         */
        
    };
    
    void push(ContextAwareDataOutput);
    
    template <typename T>
    TypeSerializer* serializerFor(){
        
    };
    
//    template <typename T>//TODO i added template
    TypeSerializer* serializerFor(int const);
    
    SerializationContext* getSerializationContext();
    
private:
    
    ContextAwareDataOutput pop();
    
//    template <typename T>//TODO i added template
    void registerDefault(std::string, TypeSerializer*);
    
    int indexForDefaultType(int const);
    
//    static long combineToLong(int x, int y);
//    static int extractInt(long value, bool lowerBits);
    
    static int const OUTPUT_STREAM_BUFFER_SIZE = 32 * 1024;
    static int const CONSTANT_SERIALIZERS_SIZE = SerializationConstants::CONSTANT_SERIALIZERS_LENGTH;
    
    map<std::string, TypeSerializer*> constantsTypeMap;
    TypeSerializer** constantTypeIds ;
    
//    queue<ContextAwareDataOutput> outputPool;
    PortableSerializer* portableSerializer;
    SerializationContext* serializationContext;
    

};
#endif /* defined(__Server__SerializationServiceImpl__) */
