//
//  TestInnerPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestInnerPortable_h
#define Server_TestInnerPortable_h


#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/Array.h"
#include "TestNamedPortable.h"
#include <iostream>
using namespace hazelcast::client;

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
                        Array<double> d,
                        Array<Portable*> n):bb(b),cc(c),ss(s),ii(i),ll(l),ff(f),dd(d),nn(n){
        
        
    };
    
    TestInnerPortable& operator=(const TestInnerPortable& rhs){
        bb = rhs.bb;
        cc = rhs.cc;
        ss = rhs.ss;
        ii = rhs.ii;
        ll = rhs.ll;
        ff = rhs.ff;
        dd = rhs.dd;
        nn = rhs.nn;
        return (*this);
    }    
    int getClassId() {
        return 1;
    };
    ~TestInnerPortable(){
    }    
    void writePortable(serialization::PortableWriter& writer) {
        writer.writeByteArray("b", bb);
        writer.writeCharArray("c", cc);
        writer.writeShortArray("s", ss);
        writer.writeIntArray("i", ii);
        writer.writeLongArray("l", ll);
        writer.writeFloatArray("f", ff);
        writer.writeDoubleArray("d", dd);
        writer.writePortableArray("nn", nn);
        
    };
        
    void readPortable(serialization::PortableReader& reader) {
        bb = reader.readByteArray("b");
        cc = reader.readCharArray("c");
        ss = reader.readShortArray("s");
        ii = reader.readIntArray("i");
        ll = reader.readLongArray("l");
        ff = reader.readFloatArray("f");
        dd = reader.readDoubleArray("d");
        Array< auto_ptr<Portable> > temp;
        temp = reader.readPortableArray("nn");
        Array<Portable*> tempNN(temp.length());
        for(int i = 0; i < temp.length() ; i++)
            tempNN[i] = temp[i].release();
        nn = tempNN;
    };
    
    bool operator==(TestInnerPortable& m){
       if( bb != m.bb ) return false;
       if( cc != m.cc ) return false;
       if( ss != m.ss ) return false;
       if( ii != m.ii ) return false;
       if( ll != m.ll ) return false;
       if( ff != m.ff ) return false;
       if( dd != m.dd ) return false;
       for(int i = 0; i < nn.length() ; i++)
           
           if( *((TestNamedPortable*)(nn[i])) 
                   != 
                   *((TestNamedPortable*)(m.nn[i]))  ) 
                   return false;
       return true;
    };
    
    bool operator!=(TestInnerPortable& m){
        return !(*this == m );  
    };
private:
    Array<byte> bb;
    Array<char> cc;
    Array<short> ss;
    Array<int> ii;
    Array<long> ll;
    Array<float> ff;
    Array<double> dd;
    Array< Portable* > nn;
    
};
#endif
