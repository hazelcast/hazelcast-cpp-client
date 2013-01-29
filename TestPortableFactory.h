//
//  TestPortableFactory.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__TestPortableFactory__
#define __Server__TestPortableFactory__

#include "hazelcast/client/serialization/PortableFactory.h"
#include "hazelcast/client/serialization/Portable.h"
#include "TestMainPortable.h"
#include "TestInnerPortable.h"
#include "TestNamedPortable.h"
#include <iostream>
#include <memory>

using namespace hazelcast::client::serialization;

class TestPortableFactory : public PortableFactory{
    
public:
    Portable* create(int classId) {
        switch (classId) {
            case 0:
                return new TestMainPortable();
            case 1:
                return new TestInnerPortable();
            case 2:
                return new TestNamedPortable();
            default:
                throw "Illegal Statement Exception";
            }
            
    };
    
};
#endif /* defined(__Server__TestPortableFactory__) */
