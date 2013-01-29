//
//  PortableFactory.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_FACTORY
#define HAZELCAST_PORTABLE_FACTORY

#include "Portable.h"
#include <iostream>
#include <memory>

namespace hazelcast{ 
namespace client{
namespace serialization{

class PortableFactory{
public:
    virtual Portable* create(int) = 0;
};

}}}
#endif /* HAZELCAST_PORTABLE_FACTORY */
