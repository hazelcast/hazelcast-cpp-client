//
//  TestInnerPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestInnerPortable_h
#define Server_TestInnerPortable_h


#include <iostream>
#include "PortableReader.h"
#include "PortableWriter.h"
#include "Portable.h"
#include "PortablePointerArray.h"
#include "TestNamedPortable.h";
using namespace std;

class TestInnerPortable : public Portable{
public:
    TestInnerPortable() {
    };
        
    TestInnerPortable(ByteArray* b, PortablePointerArray* n, char* c , short* s, int* i , long* l, float* f, double* d,
                                                              int cLen, int sLen, int iLen, int lLen, int fLen, int dLen) {
        bb = b;
        nn = n;
        cc = c;
        ss = s;
        ii = i;
        ll = l;
        ff = f;
        dd = d;
        ccLen = cLen;
        ssLen = sLen;
        iiLen = iLen;
        llLen = lLen;
        ffLen = fLen;
        ddLen = dLen;
    };
        
    int getClassId() {
        return 1;
    };
    ~TestInnerPortable(){
        delete bb;
        delete [] cc;
        delete [] ss;
        delete [] ii;
        delete [] ll;
        delete [] ff;
        delete [] dd;
        delete nn;
    }    
    void writePortable(PortableWriter* writer) throw(std::ios_base::failure){
        writer->writeByteArray("b", *bb);
        writer->writeCharArray("c", cc,ccLen);
        writer->writeShortArray("s", ss,ssLen);
        writer->writeIntArray("i", ii, iiLen);
        writer->writeLongArray("l", ll, llLen);
        writer->writeFloatArray("f", ff, ffLen);
        writer->writeDoubleArray("d", dd, ddLen);
        writer->writePortableArray("n", *nn);
        
    };
        
    void readPortable(PortableReader* reader)throw(std::ios_base::failure) {
        bb = reader->readByteArray("b");
        cc = reader->readCharArray("c",ccLen);
        ss = reader->readShortArray("s",ssLen);
        ii = reader->readIntArray("i", iiLen);
        ll = reader->readLongArray("l", llLen);
        ff = reader->readFloatArray("f", ffLen);
        dd = reader->readDoubleArray("d", ddLen);
        nn = reader->readPortableArray("n");
        
    };
    
    bool operator==(TestInnerPortable& m){
       if( !((*bb) == *(m.bb)) ) return false;
       if(memcmp(cc,m.cc,ccLen)) return false;
       if(memcmp(ss,m.ss,ssLen)) return false;
       if(memcmp(ii,m.ii,iiLen)) return false;
       if(memcmp(ll,m.ll,llLen)) return false;
       if(memcmp(ff,m.ff,ffLen)) return false;
       if(memcmp(dd,m.dd,ddLen)) return false;
//       if( !((*nn) == *(m.nn)) ) return false; TODO may need to add template to PortablePointerArray 
    };
    
    bool operator!=(TestInnerPortable& m){
        return !(*this == m );  
    };
private:
    ByteArray* bb;
    
    char* cc;
    int ccLen;
    
    short* ss;
    int ssLen;
    
    int* ii;
    int iiLen;
    
    long* ll;
    int llLen;
    
    float* ff;
    int ffLen;
    
    double* dd;
    int ddLen;
    
    PortablePointerArray* nn;
    
};
#endif
