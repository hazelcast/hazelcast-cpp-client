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
#include "DataInput.h"
#include "DataOutput.h"
#include "DefaultPortableWriter.h"
#include "PortableReader.h"
#include "PortableReader.h"
#include "MorphingPortableReader.h"

PortableSerializer::PortableSerializer(SerializationContext* context):context(context){

};

int PortableSerializer::getTypeId(){
    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
};

PortableSerializer::~PortableSerializer(){

};


ClassDefinition* PortableSerializer::getClassDefinition(Portable& p) throw(std::ios_base::failure) {
    int classId = p.getClassId();
    ClassDefinition* cd;
    
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

int PortableSerializer::getVersion(){
    return context->getVersion();
};

void PortableSerializer::write(DataOutput* dataOutput, Portable& p) throw(std::ios_base::failure) {
    
    ClassDefinition* cd = getClassDefinition(p);
    DefaultPortableWriter writer(this, dataOutput, cd);
    p.writePortable(writer);
    
};

auto_ptr<Portable> PortableSerializer::read(DataInput& dataInput) throw(std::ios_base::failure){
    
    int dataClassId = dataInput.getDataClassId();
    int dataVersion = dataInput.getDataVersion();
    auto_ptr<Portable> p = context->createPortable(dataClassId);
    
    ClassDefinition* cd;
    if (context->getVersion() == dataVersion) {
        cd = context->lookup(dataClassId); // using context.version
        PortableReader reader(this, dataInput, cd);
        p->readPortable(reader);
    } else {
        cd = context->lookup(dataClassId, dataVersion); // registered during read
        MorphingPortableReader reader(this, dataInput, cd);
        p->readPortable(reader);
    }
    return p;
    
};

