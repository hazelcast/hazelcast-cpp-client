//
// Created by sancar koyunlu on 5/8/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __CustomSerializer_H_
#define __CustomSerializer_H_

#include <iostream>
#include "TypeSerializer.h"
#include "DataOutput.h"
#include "DataInput.h"

using namespace hazelcast::client::serialization;

class CustomSerializer : public TypeSerializer {

public:
    virtual int getTypeId();

    virtual void write(DataOutput *out, void *object);

    virtual void *read(DataInput& in);
};


#endif //__CustomSerializer_H_
