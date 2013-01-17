//
//  ClassDefinitionImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <cassert>
#include "ClassDefinitionImpl.h"
#include "FieldDefinitionImpl.h"

ClassDefinitionImpl::ClassDefinitionImpl(){
    
};

void ClassDefinitionImpl::add(FieldDefinitionImpl* fd){
    assert(fd != NULL);
    fieldDefinitions.push_back(fd);
    fieldDefinitionsMap[fd->fieldName] = fd;
};

void ClassDefinitionImpl::add(ClassDefinitionImpl* cd){
    assert(cd != NULL);
    nestedClassDefinitions.insert(cd);
};

FieldDefinitionImpl* ClassDefinitionImpl::get(std::string name){
    return fieldDefinitionsMap[name];
};

FieldDefinitionImpl* ClassDefinitionImpl::get(int fieldIndex){
    return fieldDefinitions[fieldIndex];
};

set<ClassDefinitionImpl*>& ClassDefinitionImpl::getNestedClassDefinitions(){
    return nestedClassDefinitions;
};

void ClassDefinitionImpl::writeData(DataOutput & out) const throw(std::ios_base::failure){
    out.writeInt(classId);
    out.writeInt(version);
    out.writeInt((int)fieldDefinitions.size());
    for (vector<FieldDefinitionImpl*>::const_iterator it = fieldDefinitions.begin() ;
         it != fieldDefinitions.end(); it++)
        (*it)->writeData(out);
    out.writeInt((int)nestedClassDefinitions.size());
    for (set<ClassDefinitionImpl*>::iterator it = nestedClassDefinitions.begin() ; it != nestedClassDefinitions.end(); it++)
        (*it)->writeData(out);
};

void ClassDefinitionImpl::readData(DataInput & in) throw(std::ios_base::failure){
    classId = in.readInt();
    version = in.readInt();
    int size = in.readInt();
    for (int i = 0; i < size; i++) {
        FieldDefinitionImpl* fieldDefinition = new FieldDefinitionImpl();
        fieldDefinition->readData(in);
        add(fieldDefinition);
    }
    size = in.readInt();
    for (int i = 0; i < size; i++) {
        ClassDefinitionImpl* classDefinition = new ClassDefinitionImpl();
        classDefinition->readData(in);
        add(classDefinition);
    }
};

int ClassDefinitionImpl::getFieldCount(){
    return (int)fieldDefinitions.size();
};

int ClassDefinitionImpl::getClassId(){
    return classId;
};

int ClassDefinitionImpl::getVersion(){
    return version;
};

byte* ClassDefinitionImpl::getBinary(){
    return binary;
};

void ClassDefinitionImpl::setBinary(byte *binary){
    this->binary = binary;
};

