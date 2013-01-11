//
//  ClassDefinitionImpl.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ClassDefinitionImpl__
#define __Server__ClassDefinitionImpl__

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <set>
#include "ClassDefinition.h"
#include "ClassDefinitionImpl.h"
#include "DataSerializable.h"
#include "FieldDefinitionImpl.h"

using namespace std;

class ClassDefinitionImpl : public ClassDefinition{
public:
    ClassDefinitionImpl(){};//TODO 
    
    void add(FieldDefinitionImpl&);
    void add(ClassDefinitionImpl&);
    
    FieldDefinition get(std::string){};
    FieldDefinition get(int){};
    
    set<ClassDefinitionImpl> getNestedClassDefinitions();
    
    void writeData(DataOutput&)const throw(std::ios_base::failure){};
    void readData(DataInput&)throw(std::ios_base::failure){};
    
    int getFieldCount(){};
    int getClassId(){};
    int getVersion(){};
    byte* getBinary(){};
    
    void setBinary(byte* binary);
    
//    bool operator==(const ClassDefinitionImpl&) const;
//    bool operator!=(const ClassDefinitionImpl&) const;
    
//    friend std::ostream& operator<<(std::ostream&, const FieldDefinition&);
private:
    
    int classId;
    int version;
    
    list<FieldDefinitionImpl*> fieldDefinitions;
    map<std::string, FieldDefinitionImpl*> fieldDefinitionsMap;
    set<ClassDefinitionImpl*> nestedClassDefinitions;
    
    byte* binary;
    
};
#endif /* defined(__Server__ClassDefinitionImpl__) */
