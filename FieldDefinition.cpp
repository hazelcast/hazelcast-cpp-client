//
//  FieldDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <string>
#include <iostream>
#include "FieldDefinition.h"
#include "DataOutput.h"
#include "DataInput.h"
FieldDefinition::FieldDefinition(){
    
};

FieldDefinition::FieldDefinition(int index,std::string fieldName, byte type):classId(-1){
    this->index = index;
    this->fieldName = fieldName;
    this->type = type;
};

FieldDefinition::FieldDefinition(int index,std::string fieldName, byte type,int classId){
    this->index = index;
    this->fieldName = fieldName;
    this->type = type;
    this->classId = classId;
};

byte FieldDefinition::getType(){
    return type;
};

std::string FieldDefinition::getName(){
    return fieldName;
};

int FieldDefinition::getIndex(){
    return index;
};

int FieldDefinition::getClassId(){
    return classId;
};

void FieldDefinition::writeData(DataOutput& out) const throw(std::ios_base::failure){
    out.writeInt(index);
    out.writeUTF(fieldName);
    out.writeByte(type);
    out.writeInt(classId);
};

void FieldDefinition::readData(DataInput& in) throw(std::ios_base::failure){
    index = in.readInt();
    fieldName = in.readUTF();
    type = in.readByte();
    classId = in.readInt();
};

bool FieldDefinition::operator==(const FieldDefinition & other)const{
    if(this == &other) return true;
    
    if (classId != other.classId) return false;
    if (index != other.index) return false;
    if (type != other.type) return false;
    if (fieldName.compare(other.fieldName)) return false;
    
    return true;
};

bool FieldDefinition::operator!=(const FieldDefinition & other)const{
    return !(*this == other);
};