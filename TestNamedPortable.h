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
        
    TestNamedPortable(string name):name(name) {
        
    };
    
    TestNamedPortable(const TestNamedPortable& rhs){
        *this = rhs;
    }
    
    const TestNamedPortable& operator=(const TestNamedPortable& rhs){
        name = rhs.name;
    };
    
    int getClassId() {
        return 2;
    };
        
    void writePortable(PortableWriter& writer) throw(std::ios_base::failure){
        writer.writeUTF("name",name); 
    };
        
    void readPortable(PortableReader& reader) throw(std::ios_base::failure) {      
        name = reader.readUTF("name");
    };
    
    bool operator==(TestNamedPortable& m){
        if(this == &m)
            return true;
        if (name.compare(m.name)) return false;
        return true;
    };
    
    bool operator!=(TestNamedPortable& m){
        return !(*this == m );  
    };
    std::string name;
};
#endif
