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
    TestInnerPortable(std::vector<byte> b,  
                        std::vector<char> c , 
                        std::vector<short> s, 
                        std::vector<int>  i , 
                        std::vector<long> l, 
                        std::vector<float> f, 
                        std::vector<double> d,
                        std::vector<TestNamedPortable> n):bb(b),cc(c),ss(s),ii(i),ll(l),ff(f),dd(d),nn(n){
        
        
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
        return 2;
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
        std::vector<Portable*> p(nn.size());
        for(int i = 0 ; i < nn.size() ; i++)
            p[i] = &(nn[i]);
        writer.writePortableArray("nn", p);
        
    };
        
    void readPortable(serialization::PortableReader& reader) {
        bb = reader.readByteArray("b");
        cc = reader.readCharArray("c");
        ss = reader.readShortArray("s");
        ii = reader.readIntArray("i");
        ll = reader.readLongArray("l");
        ff = reader.readFloatArray("f");
        dd = reader.readDoubleArray("d");
        nn = reader.readPortableArray<TestNamedPortable>("nn");
    };
    
    bool operator==(TestInnerPortable& m){
       if( bb != m.bb ) return false;
       if( cc != m.cc ) return false;
       if( ss != m.ss ) return false;
       if( ii != m.ii ) return false;
       if( ll != m.ll ) return false;
       if( ff != m.ff ) return false;
       if( dd != m.dd ) return false;
       for(int i = 0; i < nn.size() ; i++)
           if( nn[i] != m.nn[i]  ) 
                return false;
       return true;
    };
    
    bool operator!=(TestInnerPortable& m){
        return !(*this == m );  
    };
private:
    std::vector<byte> bb;
    std::vector<char> cc;
    std::vector<short> ss;
    std::vector<int> ii;
    std::vector<long> ll;
    std::vector<float> ff;
    std::vector<double> dd;
    std::vector< TestNamedPortable > nn;
    
};
#endif
