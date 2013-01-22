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
#include <memory>
#include "DataSerializable.h"
#include "ClassDefinitionImpl.h"
#include "Array.h"
class Data{ //:  public DataSerializable{ //TODO skipped probaly not used
public:
    Data();
    Data(const Data&);
    Data(const int type, Array<byte> bytes);
    Data& operator=(const Data&);
    int size();
    
    int getPartitionHash();
    void setPartitionHash(int partitionHash);
    
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;
    
    
    ClassDefinitionImpl cd;
    int type;
    Array<byte> buffer;
private:
    int partitionHash;
    
};

#endif /* defined(__Server__Data__) */
