//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "Data.h"

Data::Data(){
    
};

Data::Data(int type, byte* bytes){
    
};

int Data::size(){
    
};

void Data::writeData(DataOutput&) const throw(std::ios_base::failure){
    
};

void Data::readData(DataInput&) throw(std::ios_base::failure){
    
};

int Data::getPartitionHash(){
    return partitionHash;
};

void Data::setPartitionHash(int partitionHash){
    
};


//bool Data::operator==(const FieldDefinitionImpl&) const{

//};

//bool Data::operator!=(const FieldDefinitionImpl&) const{

//};

// Same as Arrays.equals(byte[] a, byte[] a2) but loop order is reversed.
 bool Data::equals(byte const * const data1, byte const * const data2){
    
};
