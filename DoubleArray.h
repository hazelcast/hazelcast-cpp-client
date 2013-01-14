//
//  DoubleArray.h
//  cppClient
//
//  Created by sancar koyunlu on 1/14/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef cppClient_DoubleArray_h
#define cppClient_DoubleArray_h

class DoubleArray{
public:
    DoubleArray(int len):len(0){
        this->len = len;
        buffer = new double[len];
    };
    ~DoubleArray(){
        delete [] buffer;
    };
    double& operator[](int i){
        return buffer[i];
    };
    
    int length(){ return len; };
private:
    int len;
    double* buffer;
};

#endif
