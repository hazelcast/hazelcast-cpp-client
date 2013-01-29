//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "DataSerializable.h"
#include "../Array.h"
#include <iostream>
#include <memory>

namespace hazelcast{ 
namespace client{
namespace serialization{

class DataOutput;
class DataInput;
class ClassDefinition;

typedef unsigned char byte;

class Data : public DataSerializable{ 
public:
    Data();
    Data(const Data&);
    Data(const int type, Array<byte> bytes);
    Data& operator=(const Data&);
    int size() const;
    
    int getPartitionHash();
    void setPartitionHash(int partitionHash);
    
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
    
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&) throw(std::ios_base::failure);
    
    ClassDefinition* cd;
    int type;
    Array<byte> buffer;
private:
    int partitionHash;
    static const int NO_CLASS_ID = -1;
    
};

}}}
#endif /* defined(__Server__Data__) */
