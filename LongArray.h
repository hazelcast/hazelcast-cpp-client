//
//  LongArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_LongArray_h
#define cppClient_LongArray_h

class LongArray{
public:
    LongArray(int len):len(0){
        this->len = len;
        buffer = new long[len];
    };
    LongArray(const LongArray& rhs){
        len = rhs.len;
        delete [] buffer;
        buffer = new long[len];
        for(int i = 0; i < len; i++){
            buffer[i] = rhs.buffer[i];
        }
    };
    ~LongArray(){
        delete [] buffer;
    };
    long& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    long* buffer;
};

#endif
