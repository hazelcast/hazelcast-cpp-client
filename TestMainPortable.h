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

class TestMainPortable : public Portable{
public:
    int i;
    
        
    TestMainPortable() {
    };
        
    TestMainPortable(int i) {
        this->i = i;
    };
        
    int getClassId() {
        return 50;
    };
        
    void writePortable(PortableWriter* writer) throw(std::ios_base::failure){
        writer->writeInt("i",i);
    };
        
    void readPortable(PortableReader* reader)throw(std::ios_base::failure) {
        i = reader->readInt("i");
    };
    
    int getType(){
        return 50;
    };
    
    
};
#endif
