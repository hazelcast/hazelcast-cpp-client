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

class TypeSerializer{
public:
    template<typename T>
    inline void write(DataOutput*, T ) throw(std::ios_base::failure){
        
    };
    
    template<typename T>
    inline T read(DataInput*) throw(std::ios_base::failure){
        
    };
    
};

#endif
