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
    CharArray(){
        
    };
    CharArray(int len){
        this->len = len;
        buffer = new char[len];
    };
    CharArray(const CharArray& rhs){
        len = rhs.len;
        delete [] buffer;
        buffer = new char[len];
        for(int i = 0; i < len; i++){
            buffer[i] = rhs.buffer[i];
        }
    };
    ~CharArray(){
        delete [] buffer;
    };
    char& operator[](int i){
        return buffer[i];
    };
    bool operator==(CharArray& m){
        if(len != m.len)
            return false;
        for(int i = 0; i < len ; i++){
            if(m[i] != buffer[i])
                return false;
        }
        return true;
    };
    int length(){ return len; };
private:
    int len;
    char* buffer;
};

#endif
