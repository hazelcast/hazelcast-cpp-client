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
#include <memory>
#include "PortableFactory.h"
#include "Portable.h"
#include "TestMainPortable.h";
#include "TestInnerPortable.h";
#include "TestNamedPortable.h";

class TestPortableFactory : public PortableFactory{
    
public:
    auto_ptr<Portable> create(int classId) {
        switch (classId) {
            case 0:
                return auto_ptr<Portable>(new TestMainPortable());
            case 1:
                return auto_ptr<Portable>(new TestInnerPortable());
            case 2:
                return auto_ptr<Portable>(new TestNamedPortable());
            default:
                throw "Illegal Statement Exception";
            }
            
    };
    
};
#endif /* defined(__Server__TestPortableFactory__) */
