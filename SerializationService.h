//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_SerializationService_h
#define Server_SerializationService_h

class Data;
class SerializationContextImpl;//TODO change into SerializationContext

class SerializationService{
public:
    template<typename K>
    Data* toData(K&);//TODO virtual ?
    
    template<typename K>
    K toObject(Data*);//TODO virtual ?
    
    virtual SerializationContextImpl* getSerializationContext() = 0;
};

#endif
