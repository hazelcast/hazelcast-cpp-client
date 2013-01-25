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

ClassDefinitionImpl::ClassDefinitionImpl():classId(-1),version(-1){
    
};

ClassDefinitionImpl::ClassDefinitionImpl(const ClassDefinitionImpl& rhs){
    classId = rhs.classId;
    version = rhs.version;
    fieldDefinitions = rhs.fieldDefinitions;
    fieldDefinitionsMap = rhs.fieldDefinitionsMap;
    nestedClassDefinitions = rhs.nestedClassDefinitions;
    binary = rhs.binary;
};

ClassDefinitionImpl& ClassDefinitionImpl::operator=(const ClassDefinitionImpl& rhs){
    classId = rhs.classId;
    version = rhs.version;
    fieldDefinitions = rhs.fieldDefinitions;
    fieldDefinitionsMap = rhs.fieldDefinitionsMap;
    nestedClassDefinitions = rhs.nestedClassDefinitions;
    binary = rhs.binary;
    return (*this);
};

void ClassDefinitionImpl::add(FieldDefinitionImpl& fd){
    fieldDefinitions.push_back(fd);
    fieldDefinitionsMap[fd.fieldName] = fd;
};

void ClassDefinitionImpl::add(ClassDefinitionImpl* cd){
    nestedClassDefinitions.push_back(cd);
};

bool ClassDefinitionImpl::isFieldDefinitionExists(std::string name){
    return (fieldDefinitionsMap.count(name) > 0); 
}

const FieldDefinitionImpl& ClassDefinitionImpl::get(std::string name){
    return fieldDefinitionsMap[name];
};

const FieldDefinitionImpl& ClassDefinitionImpl::get(int fieldIndex){
    return fieldDefinitions[fieldIndex];
};

const vector<ClassDefinitionImpl*>& ClassDefinitionImpl::getNestedClassDefinitions(){
    return nestedClassDefinitions;
};

void ClassDefinitionImpl::writeData(DataOutput & out) const throw(std::ios_base::failure){
    out.writeInt(classId);
    out.writeInt(version);
    out.writeInt((int)fieldDefinitions.size());
    for (vector<FieldDefinitionImpl>::const_iterator it = fieldDefinitions.begin() ;  it != fieldDefinitions.end(); it++)
        (*it).writeData(out);
    out.writeInt((int)nestedClassDefinitions.size());
    for (vector<ClassDefinitionImpl*>::const_iterator it = nestedClassDefinitions.begin() ; it != nestedClassDefinitions.end(); it++)
        (*it)->writeData(out);
};

void ClassDefinitionImpl::readData(DataInput & in) throw(std::ios_base::failure){
    classId = in.readInt();
    version = in.readInt();
    int size = in.readInt();
    for (int i = 0; i < size; i++) {
        FieldDefinitionImpl fieldDefinition;
        fieldDefinition.readData(in);
        add(fieldDefinition);
    }
    size = in.readInt();
    for (int i = 0; i < size; i++) {
        ClassDefinitionImpl* classDefinition = new ClassDefinitionImpl;
        classDefinition->readData(in);
        add(classDefinition);
    }
};

int ClassDefinitionImpl::getFieldCount(){
    return (int)fieldDefinitions.size();
};

int ClassDefinitionImpl::getClassId() const{
    return classId;
};

int ClassDefinitionImpl::getVersion() const{
    return version;
};

Array<byte> ClassDefinitionImpl::getBinary() const{
    return binary;
};

void ClassDefinitionImpl::setBinary(Array<byte>& binary){
    this->binary = binary;
};

bool ClassDefinitionImpl::operator ==(const ClassDefinitionImpl& rhs) const{
    if(this == &rhs) return true;
    if(classId != rhs.classId) return false;
    if(version != rhs.version) return false;
    if(fieldDefinitions != rhs.fieldDefinitions) return false;
    if(fieldDefinitionsMap != rhs.fieldDefinitionsMap) return false;
    if(nestedClassDefinitions != rhs.nestedClassDefinitions) return false;
    if(binary != rhs.binary) return false;
    return true;
};

bool ClassDefinitionImpl::operator !=(const ClassDefinitionImpl& rhs) const{
    return !(*this == rhs);
};