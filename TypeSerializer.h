//
//  TypeSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TypeSerializer_h
#define Server_TypeSerializer_h

class DataInput;
class DataOutput;

template<typename T>
class TypeSerializer{
public:
    void write(DataOutput*, T ) throw(std::ios_base::failure);
    
    T read(DataInput*) throw(std::ios_base::failure);

    
};

#endif
