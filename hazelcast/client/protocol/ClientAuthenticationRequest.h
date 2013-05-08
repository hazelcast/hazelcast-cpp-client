//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef __ClientAuthenticationRequest_H_
#define __ClientAuthenticationRequest_H_

#include <iostream>
#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"


class ClientAuthenticationRequest : public hazelcast::client::serialization::Portable{

    public:

    virtual int getFactoryId();

    virtual int getClassId();

    virtual void writePortable(hazelcast::client::serialization::PortableWriter & writer);

    virtual void readPortable(hazelcast::client::serialization::PortableReader & reader);
};


#endif //__ClientAuthenticationRequest_H_
