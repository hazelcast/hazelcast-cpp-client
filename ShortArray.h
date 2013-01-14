//
//  ShortArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_ShortArray_h
#define cppClient_ShortArray_h

class ShortArray{
public:
    ShortArray(int len):len(0){
        this->len = len;
        buffer = new short[len];
    };
    ~ShortArray(){
        delete [] buffer;
    };
    short& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    short* buffer;
};

#endif
