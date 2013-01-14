//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_SerializationService_h
#define Server_SerializationService_h

#include "Data.h"
#include "SerializationContext.h"

class SerializationService{
public:
    template<typename K>
    Data toData(K);//TODO virtual ?
    
    template<typename K>
    K toObject(Data*);//TODO virtual ?
    
    virtual SerializationContext* getSerializationContext() = 0;
};

#endif
