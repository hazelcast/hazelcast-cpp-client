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

using namespace std;

class TestMainPortable : public Portable{
public:
    int i;
    float f;
        
    TestMainPortable() {
    };
        
    TestMainPortable(int i, float f) {
        this->i = i;
        this->f = f;
    };
        
    int getClassId() {
        return 50;
    };
        
    void writePortable(PortableWriter* writer) throw(std::ios_base::failure){
        writer->writeInt("myInteger",i);
        writer->writeFloat("myFloat",f );
        
    };
        
    void readPortable(PortableReader* reader)throw(std::ios_base::failure) {
        f = reader->readFloat("myFloat");
        i = reader->readInt("myInteger");
    };
    
    bool operator==(TestMainPortable& m){
        return (i == m.i) && (f == m.f );
    };
    
    bool operator!=(TestMainPortable& m){
        return !(*this == m );  
    };
    
};
#endif
