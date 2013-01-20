//
//  PortableFactory.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__PortableFactory__
#define __Server__PortableFactory__

#include <iostream>
#include "Portable.h"

class PortableFactory{
public:
    virtual Portable create(int) = 0;
};
#endif /* defined(__Server__PortableFactory__) */
