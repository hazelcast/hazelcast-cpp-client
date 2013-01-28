//
//  TestMainPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestMainPortable_h
#define Server_TestMainPortable_h


#include <iostream>
#include "PortableReader.h"
#include "PortableWriter.h"
#include "Portable.h"
#include "TestInnerPortable.h"
using namespace std;

class TestMainPortable : public Portable{
public:
        
    TestMainPortable() :p(NULL){
    };
    
    TestMainPortable(const TestMainPortable& rhs){
        *this = rhs;
    }
    ~TestMainPortable(){
        delete p;
    }
    TestMainPortable(byte b, bool boolean, char c, short s, int i, long l, float f, double d, string str, TestInnerPortable* p) {
        this->b = b;
        this->boolean = boolean;
        this->c = c;
        this->s = s;
        this->i = i;
        this->l = l;
        this->f = f;
        this->d = d;
        this->str = str;
        this->p = new TestInnerPortable(*p);
    };
    
    const TestMainPortable& operator=(const TestMainPortable& rhs){
        b = rhs.b;
        boolean = rhs.boolean;
        c = rhs.c;
        s = rhs.s;
        i = rhs.i;
        l = rhs.l;
        f = rhs.f;
        d = rhs.d;
        str = rhs.str;
        p = new TestInnerPortable(*rhs.p);
        return (*this);
    };
    
    
    int getClassId() {
        return 0;
    };
        
    void writePortable(PortableWriter& writer) throw(std::ios_base::failure){
        writer.writeByte("b", b);
        
        writer.writeBoolean("bool", boolean);
        writer.writeChar("c", c);
        writer.writeShort("s", s);
        writer.writeInt("i", i);
        writer.writeLong("l", l);
        writer.writeFloat("f", f);
        writer.writeDouble("d", d);
        writer.writeUTF("str", str);
        writer.writePortable("p", *p);
        
    };
        
    void readPortable(PortableReader& reader)throw(std::ios_base::failure) {
        b = reader.readByte("b");
        boolean = reader.readBoolean("bool");
        c = reader.readChar("c");
        s = reader.readShort("s");
        i = reader.readInt("i");
        l = reader.readLong("l");
        f = reader.readFloat("f");
        d = reader.readDouble("d");
        str = reader.readUTF("str");
        auto_ptr<TestInnerPortable> p_ptr = reader.readPortable<TestInnerPortable>("p");
        p = p_ptr.release();
    };
    
    bool operator==(TestMainPortable& m){
        if(this == &m) return true;
        
        if(b != m.b ) return false;
        if(boolean != m.boolean ) return false;
        if(c != m.c ) return false;
        if(s != m.s ) return false;
        if(i != m.i ) return false;
        if(l != m.l ) return false;
        if(f != m.f ) return false;
        if(d != m.d ) return false;
        if( str.compare(m.str) ) return false;
        if(*p != *(m.p) ) return false;
        return true;
    };
    
    bool operator!=(TestMainPortable& m){
        return !(*this == m );  
    };
    TestInnerPortable* p;
private:
    byte b;
    bool boolean;
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    string str;
};
#endif
