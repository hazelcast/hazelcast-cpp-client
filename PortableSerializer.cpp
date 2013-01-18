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

PortableSerializer::PortableSerializer(SerializationContextImpl* context){
    this->context = context;
};

int PortableSerializer::getTypeId(){
    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
};

PortableSerializer::~PortableSerializer(){
    //TODO sda 
};


ClassDefinitionImpl* PortableSerializer::getClassDefinition(Portable& p) throw(std::ios_base::failure) {
    int classId = p.getClassId();
    ClassDefinitionImpl* cd = context->lookup(classId);
    if (cd == NULL) {
        ClassDefinitionWriter* classDefinitionWriter = new ClassDefinitionWriter(classId,context->getVersion(),this);
        assert(classDefinitionWriter != NULL);
        p.writePortable(classDefinitionWriter);
        cd = &classDefinitionWriter->cd;
        context->registerClassDefinition(cd);
    }
    assert(cd != NULL);
    return cd;
};

SerializationContextImpl* PortableSerializer::getContext(){
    return context;
};//TODO propbaly unused check later

int PortableSerializer::getVersion(){
    return context->getVersion();
};

void PortableSerializer::write(ContextAwareDataOutput* dataOutput, Portable& p) throw(std::ios_base::failure) {
    
    ClassDefinitionImpl* cd = getClassDefinition(p);
    assert(cd != NULL);
    DefaultPortableWriter* writer = new DefaultPortableWriter(this, dataOutput, cd);
    p.writePortable(writer);
    
};

Portable* PortableSerializer::read(ContextAwareDataInput* dataInput) throw(std::ios_base::failure){
    assert(dataInput != NULL);
    int dataClassId = dataInput->getDataClassId();
    int dataVersion = dataInput->getDataVersion();
    Portable* p = context->createPortable(dataClassId);
    assert(p != NULL);
    DefaultPortableReader* reader;
    ClassDefinitionImpl* cd;
    if (context->getVersion() == dataVersion) {
        cd = context->lookup(dataClassId); // using context.version
        assert(cd != NULL);
        reader = new DefaultPortableReader(this, dataInput, cd);
    } else {
        cd = context->lookup(dataClassId, dataVersion); // registered during read
//        reader = new MorphingPortableReader(this, (BufferObjectDataInput) dataInput, cd);
    }
    p->readPortable(reader);
    return p;
    
};

