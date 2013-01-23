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
#include "TestNamedPortable.h"
#include "Array.h"
using namespace std;

class TestInnerPortable : public Portable{
public:
    TestInnerPortable(){
    };
    TestInnerPortable(const TestInnerPortable& rhs){    
        *this = rhs;
    }
    TestInnerPortable(Array<byte> b,  
                        Array<char> c , 
                        Array<short> s, 
                        Array<int>  i , 
                        Array<long> l, 
                        Array<float> f, 
                        Array<double> d
                        ):bb(b),cc(c),ss(s),ii(i),ll(l),ff(f),dd(d){
        
        
    };
    
    TestInnerPortable& operator=(const TestInnerPortable& rhs){
        bb = rhs.bb;
        cc = rhs.cc;
        ss = rhs.ss;
        ii = rhs.ii;
        ll = rhs.ll;
        ff = rhs.ff;
        dd = rhs.dd;
//        nn = rhs.nn;
        return (*this);
    }    
    int getClassId() {
        return 1;
    };
    ~TestInnerPortable(){
    }    
    void writePortable(PortableWriter& writer) throw(std::ios_base::failure){
        writer.writeByteArray("b", bb);
        writer.writeCharArray("c", cc);
        writer.writeShortArray("s", ss);
        writer.writeIntArray("i", ii);
        writer.writeLongArray("l", ll);
        writer.writeFloatArray("f", ff);
        writer.writeDoubleArray("d", dd);
//        writer.writePortableArray("n", nn);
        
    };
        
    void readPortable(PortableReader& reader)throw(std::ios_base::failure) {
        bb = reader.readByteArray("b");
        cc = reader.readCharArray("c");
        ss = reader.readShortArray("s");
        ii = reader.readIntArray("i");
        ll = reader.readLongArray("l");
        ff = reader.readFloatArray("f");
        dd = reader.readDoubleArray("d");
//        nn = reader.readPortableArray("n");
        
    };
    
    bool operator==(TestInnerPortable& m){
       if( bb != m.bb ) return false;
       if( cc != m.cc ) return false;
       if( ss != m.ss ) return false;
       if( ii != m.ii ) return false;
       if( ll != m.ll ) return false;
       if( ff != m.ff ) return false;
       if( dd != m.dd ) return false;
//       if( nn != m.nn ) return false;
       return true;
    };
    
    bool operator!=(TestInnerPortable& m){
        return !(*this == m );  
    };
    Array<byte> bb;
    Array<char> cc;
    Array<short> ss;
    Array<int> ii;
    Array<long> ll;
    Array<float> ff;
    Array<double> dd;
private:
//    Array< Portable* > nn;
    
};
#endif
