//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __Credentials_H_
#define __Credentials_H_

#include <iostream>
#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

typedef unsigned char byte;

class Credentials : public hazelcast::client::serialization::Portable {
public:
    Credentials() {

    };

    virtual int getFactoryId();

    virtual int getClassId();

    virtual void writePortable(hazelcast::client::serialization::PortableWriter & writer);

    virtual void readPortable(hazelcast::client::serialization::PortableReader & reader);
};


#endif //__Credentials_H_
