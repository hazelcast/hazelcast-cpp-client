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
#include <vector>
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
    Data(const int type, std::vector<byte> bytes);
    Data& operator=(const Data&);
    int bufferSize() const;
    int totalSize() const;
    
    int getPartitionHash();
    void setPartitionHash(int partitionHash);
    
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
    
    void writeData(DataOutput&) const;
    void readData(DataInput&);
    
    ClassDefinition* cd;
    int type;
    std::vector<byte> buffer;
private:
    int partitionHash;
    static const int NO_CLASS_ID = 0;
    
};

}}}
#endif /* defined(__Server__Data__) */
