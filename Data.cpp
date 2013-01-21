//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Array.h"
#include "Data.h"

Data::Data():partitionHash(-1),buffer(0),type(-1){
    
};

Data::Data(const Data& rhs){
    (*this) = rhs;    
};

Data::Data(const int type, Array<byte> buffer):partitionHash(-1){
    this->type = type;
    this->buffer = buffer;       
};

Data& Data::operator=(const Data& rhs){
    type = rhs.type;
    buffer = rhs.buffer;
    cd = rhs.cd;
    partitionHash = rhs.partitionHash;
};

bool Data::operator==(const Data& rhs) const{
    if(type != rhs.type) return false;
    if(cd != rhs.cd) return false; //TODO check this again
    if(partitionHash != rhs.partitionHash) return false;
    if(buffer != rhs.buffer) return false;
    return true;
};

bool Data::operator!=(const Data& rhs) const{
    return !((*this) == rhs);
};
    
int Data::size(){
    return buffer.length();
};


int Data::getPartitionHash(){
    return partitionHash;
};

void Data::setPartitionHash(int partitionHash){
    this->partitionHash = partitionHash;
};


