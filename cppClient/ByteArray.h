//
//  ByteArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __cppClient__ByteArray__
#define __cppClient__ByteArray__

typedef unsigned char byte;
class ByteArray{
public:
    ByteArray(int len):len(0){
        this->len = len;
        buffer = new byte[len];
    };
    ~ByteArray(){
        delete [] buffer;
    };
    byte& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    byte* buffer;
};

#endif /* defined(__cppClient__ByteArray__) */
