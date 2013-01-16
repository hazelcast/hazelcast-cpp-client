//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableSerializer.h"
#include "SerializationConstants.h"
#include "ClassDefinitionWriter.h"
/*
PortableSerializer::PortableSerializer(SerializationContext* context){
    this->context = context;
};

int PortableSerializer::getTypeId(){
    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
};

PortableSerializer::~PortableSerializer(){
    //TODO
};

ClassDefinitionImpl* PortableSerializer::getClassDefinition(Portable p) throw(std::ios_base::failure) {
    int classId = p.getClassId();
    ClassDefinitionImpl* cd = context->lookup(classId);
    if (cd == NULL) {
        ClassDefinitionWriter* classDefinitionWriter = new ClassDefinitionWriter(classId,context->getVersion());
        p.writePortable(*classDefinitionWriter);
        cd = &(classDefinitionWriter->cd);
        context->registerClassDefinition(cd);
    }
    return cd;
};

SerializationContext PortableSerializer::getContext(){

};//TODO propbaly unused check later

int PortableSerializer::getVersion(){

};

void PortableSerializer::write(DataOutput, Portable ) throw(std::ios_base::failure) {

};

Portable PortableSerializer::read(DataInput) throw(std::ios_base::failure){

};
 */
