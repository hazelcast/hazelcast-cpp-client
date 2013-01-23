//
//  FieldDefinitionImpl.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <string>
#include <iostream>
#include "FieldDefinitionImpl.h"

FieldDefinitionImpl::FieldDefinitionImpl(){
    
};

FieldDefinitionImpl::FieldDefinitionImpl(int index,std::string fieldName, byte type):classId(-1){
    this->index = index;
    this->fieldName = fieldName;
    this->type = type;
};

FieldDefinitionImpl::FieldDefinitionImpl(int index,std::string fieldName, byte type,int classId){
    this->index = index;
    this->fieldName = fieldName;
    this->type = type;
    this->classId = classId;
};

byte FieldDefinitionImpl::getType(){
    return type;
};

std::string FieldDefinitionImpl::getName(){
    return fieldName;
};

int FieldDefinitionImpl::getIndex(){
    return index;
};

int FieldDefinitionImpl::getClassId(){
    return classId;
};

void FieldDefinitionImpl::writeData(DataOutput& out) const throw(std::ios_base::failure){
    out.writeInt(index);
    out.writeUTF(fieldName);
    out.writeByte(type);
    out.writeInt(classId);
};

void FieldDefinitionImpl::readData(DataInput& in) throw(std::ios_base::failure){
    index = in.readInt();
    fieldName = in.readUTF();
    type = in.readChar();
    classId = in.readInt();
};

bool FieldDefinitionImpl::operator==(const FieldDefinitionImpl & other)const{
    if(this == &other) return true;
    
    if (classId != other.classId) return false;
    if (index != other.index) return false;
    if (type != other.type) return false;
    if (fieldName.compare(other.fieldName)) return false;
    
    return true;
};

bool FieldDefinitionImpl::operator!=(const FieldDefinitionImpl & other)const{
    return !(*this == other);
};