//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <cassert>
#include "PortableSerializer.h"
#include "SerializationConstants.h"
#include "ContextAwareDataInput.h"
#include "ContextAwareDataOutput.h"
#include "DefaultPortableWriter.h"
#include "DefaultPortableReader.h"
#include "PortableReader.h"
#include "MorphingPortableReader.h"

PortableSerializer::PortableSerializer(SerializationContextImpl* context):context(context){

};

int PortableSerializer::getTypeId(){
    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
};

PortableSerializer::~PortableSerializer(){

};


ClassDefinitionImpl PortableSerializer::getClassDefinition(Portable& p) throw(std::ios_base::failure) {
    int classId = p.getClassId();
    ClassDefinitionImpl cd;
    
    if (context->isClassDefinitionExists(classId)) {
        cd =  context->lookup(classId);
    }else{
        ClassDefinitionWriter classDefinitionWriter(classId,context->getVersion(),this);
        p.writePortable(classDefinitionWriter);
        cd = classDefinitionWriter.cd;
        context->registerClassDefinition(cd);
    }
    
    return cd;
};

SerializationContextImpl* PortableSerializer::getContext(){
    return context;
};//TODO propbaly unused check later

int PortableSerializer::getVersion(){
    return context->getVersion();
};

void PortableSerializer::write(ContextAwareDataOutput* dataOutput, Portable& p) throw(std::ios_base::failure) {
    
    ClassDefinitionImpl cd = getClassDefinition(p);
    DefaultPortableWriter writer(this, dataOutput, &cd);
    p.writePortable(writer);
    
};

auto_ptr<Portable> PortableSerializer::read(ContextAwareDataInput& dataInput) throw(std::ios_base::failure){
    
    int dataClassId = dataInput.getDataClassId();
    int dataVersion = dataInput.getDataVersion();
    auto_ptr<Portable> p = context->createPortable(dataClassId);
    
    ClassDefinitionImpl cd;
    if (context->getVersion() == dataVersion) {
        cd = context->lookup(dataClassId); // using context.version
        DefaultPortableReader reader(this, dataInput, &cd);
        p->readPortable(reader);
    } else {
        cd = context->lookup(dataClassId, dataVersion); // registered during read
        MorphingPortableReader reader(this, dataInput, &cd);
        p->readPortable(reader);
    }
    return p;
    
};

