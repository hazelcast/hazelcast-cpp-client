//
//  SerializationContextAware.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_SerializationContextAware_h
#define Server_SerializationContextAware_h
#include "SerializationContext.h"
class SerializationContextAware{
public:
    virtual SerializationContext getSerializationContext() = 0;
};

#endif
