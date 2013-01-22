//
//  TestNamedPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestNamedPortable_h
#define Server_TestNamedPortable_h


#include <iostream>
#include "PortableReader.h"
#include "PortableWriter.h"
#include "Portable.h"

using namespace std;

class TestNamedPortable : public Portable{
public:
    TestNamedPortable() {
    };
        
    TestNamedPortable(string name,short x, int y):name(name),x(x),y(y) {
        
    };
    
    TestNamedPortable(const TestNamedPortable& rhs){
        *this = rhs;
    }
    
    const TestNamedPortable& operator=(const TestNamedPortable& rhs){
        x = rhs.x;
        name = rhs.name;
        y = rhs.y;
    };
    
    int getClassId() {
        return 2;
    };
        
    void writePortable(PortableWriter& writer) throw(std::ios_base::failure){
        writer.writeInt("myInt",y);
        writer.writeUTF("name",name);
        writer.writeShort("myShort",x);
        
    };
        
    void readPortable(PortableReader& reader) throw(std::ios_base::failure) {
        y = reader.readInt("myInt");
        name = reader.readUTF("name");
        x = reader.readShort("myShort");
    };
    
    bool operator==(TestNamedPortable& m){
        if(this == &m)
            return true;
        if (name.compare(m.name)) return false;
        if ( x != m.x) return false ;
        if ( y != m.y) return false;;
    };
    
    bool operator!=(TestNamedPortable& m){
        return !(*this == m );  
    };
    std::string name;
    short x;
    int y;
private:
};
#endif
