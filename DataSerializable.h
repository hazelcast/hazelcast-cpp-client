//
//  DataSerializable.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__DataSerializable__
#define __Server__DataSerializable__

#include "DataInput.h"
#include "DataOutput.h"

typedef unsigned char byte;

class DataSerializable{
public:
    virtual void writeData(DataOutput&) const throw(std::ios_base::failure) = 0;
    virtual void readData(DataInput&) throw(std::ios_base::failure) = 0;

};

#endif /* defined(__Server__DataSerializable__) */
