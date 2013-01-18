//
//  PortableArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/16/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_PortablePointerArray_h
#define cppClient_PortablePointerArray_h
#include "Portable.h"

class PortablePointerArray{
public:
    PortablePointerArray(int len){
        this->len = len;
        buffer = new Portable*[len];
    };
    PortablePointerArray(int size, Portable** b){
        len = size;
        buffer = b;
    };
    PortablePointerArray(const PortablePointerArray& rhs){
        len = rhs.len;
        delete [] buffer;
        buffer = new Portable*[len];
        for(int i = 0; i < len; i++){
            buffer[i] = rhs.buffer[i];
        }
    };
    ~PortablePointerArray(){
        delete [] buffer;
    };
    Portable*& operator[](int i){
        return buffer[i];
    };
    bool operator==(PortablePointerArray& m){
        if(len != m.len)
            return false;
//        for(int i = 0; i < len ; i++)
//            if(!(*(m[i]) == *(buffer[i]) ))
//                return false;
        
        return true;
    };
    
    int length(){ return len; };
private:
    int len;
    Portable** buffer;
};

#endif
