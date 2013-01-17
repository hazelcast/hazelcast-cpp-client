//
//  PortableArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/16/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_PortableArray_h
#define cppClient_PortableArray_h
#include "Portable.h"
class PortableArray{
public:
    PortableArray(int len):len(0){
        this->len = len;
        buffer = new char[len];
    };
    PortableArray(const PortableArray& rhs){
        len = rhs.len;
        delete [] buffer;
        buffer = new Portable[len];
        for(int i = 0; i < len; i++){
            buffer[i] = rhs.buffer[i];
        }
    };
    ~PortableArray(){
        delete [] buffer;
    };
    char& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    Portable* buffer;
};


#endif
