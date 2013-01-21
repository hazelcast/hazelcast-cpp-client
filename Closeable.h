//
//  Closeable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_Closeable_h
#define Server_Closeable_h
#include <iostream>
class Closeable{
public:
    virtual void close() throw(std::ios_base::failure)= 0;
};

#endif
