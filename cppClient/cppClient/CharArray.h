//
//  CharArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_CharArray_h
#define cppClient_CharArray_h

class CharArray{
public:
    CharArray(int len):len(0){
        this->len = len;
        buffer = new char[len];
    };
    ~CharArray(){
        delete [] buffer;
    };
    char& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    char* buffer;
};

#endif
