//
//  TestPortableFactory.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__TestPortableFactory2__
#define __Server__TestPortableFactory2__

#include "hazelcast/client/serialization/PortableFactory.h"
#include "hazelcast/client/serialization/Portable.h"
#include "TestMainPortable.h"
#include "TestInnerPortable.h"
#include "TestNamedPortableV2.h"
#include <iostream>
#include <memory>

using namespace hazelcast::client::serialization;

class TestPortableFactory2 : public PortableFactory {

public:
    Portable *create(int classId) const {
        switch (classId) {
            case 1:
                return new TestMainPortable();
            case 2:
                return new TestInnerPortable();
            case 3:
                return new TestNamedPortablev2();
            default:
                throw "Illegal Statement Exception";
        }

    };

};

#endif /* defined(__Server__TestPortableFactory__) */
