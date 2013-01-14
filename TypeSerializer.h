//
//  TypeSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TypeSerializer_h
#define Server_TypeSerializer_h

#include "DataInput.h"
#include "DataOutput.h"


class TypeSerializer{
public:
    virtual int getTypeId() = 0;
    template<typename T>
    void write(DataOutput*, T ) throw(std::ios_base::failure);
    
    template<typename T>
    T read(DataInput) throw(std::ios_base::failure);
};

#endif
