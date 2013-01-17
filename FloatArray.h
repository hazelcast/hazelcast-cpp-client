//
//  FloatArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_FloatArray_h
#define cppClient_FloatArray_h

class FloatArray{
public:
    FloatArray(int len):len(0){
        this->len = len;
        buffer = new float[len];
    };
    FloatArray(const FloatArray& rhs){
        len = rhs.len;
        delete [] buffer;
        buffer = new float[len];
        for(int i = 0; i < len; i++){
            buffer[i] = rhs.buffer[i];
        }
    };
    ~FloatArray(){
        delete [] buffer;
    };
    float& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    float* buffer;
};

#endif
