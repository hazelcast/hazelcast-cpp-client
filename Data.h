//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__Data__
#define __Server__Data__

#include <iostream>
#include "DataSerializable.h"
#include "ClassDefinitionImpl.h"

class Data :  public DataSerializable{
public:
    Data();
    Data(int type, ByteArray* bytes);
    
    int size();
    
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&) throw(std::ios_base::failure);
    
    int getPartitionHash();
    void setPartitionHash(int partitionHash);
    
//    bool operator==(const FieldDefinitionImpl&) const;
//    bool operator!=(const FieldDefinitionImpl&) const;
    
    // Same as Arrays.equals(byte[] a, byte[] a2) but loop order is reversed.
    static bool equals(byte const * const data1, byte const * const data2);
    
    static int const NO_CLASS_ID = -1;
    
    ClassDefinitionImpl* cd;
    int type = -1;
    ByteArray* buffer;
private:
    int partitionHash;
    
};

#endif /* defined(__Server__Data__) */
