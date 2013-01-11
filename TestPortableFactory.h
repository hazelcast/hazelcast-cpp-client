//
//  TestPortableFactory.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__TestPortableFactory__
#define __Server__TestPortableFactory__

#include <iostream>
#include "PortableFactory.h"
#include "Portable.h"
#include "TestMainPortable.h"
class TestPortableFactory : public PortableFactory{
    
public:
    Portable* create(int classId) {
        return new TestMainPortable();
            
    };
    
};
#endif /* defined(__Server__TestPortableFactory__) */
