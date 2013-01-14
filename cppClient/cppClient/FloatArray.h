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
